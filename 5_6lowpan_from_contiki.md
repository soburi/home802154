TWE-Lite/Contiki から 6LoWPANする
=======================

//raw[|latex| \\thispagestyle{empty} ]


TWE-Liteを使って、ContikiOSの6LoWPANの通信を行う。

contikiのサンプルでは `examples/udp-ipv6` が通信確認に
向いたシンプルな通信プログラムになっている。
これを使って、Linux側との通信を行う。

RPL, NDP
--------
IEEE802.15.4などの省電力ネットワーク向けにRPL
(IPv6 Routing Protocol for Low-Power and Lossy Networks)
というルーティングプロトコルがあり、Contikiもこれの実装を持っている。
が、Linux側に十分こなれた実装がない。
今回は普通のv6ネットワークで使う、
NDP(Neighbor Discovery for IP version 6 (IPv6))で
Router Advatiseをradvdから通知してルーター情報の設定を行う。

NDPを使う場合、通信経路の確立のためブロードキャストのメッセージが
頻繁に流れるため、省電力ネットワークには不向きの動きとなる。
(文字通り、「寝た子を起こす」ことになるので」
現状、適当な代替手段がないので、実験的に使うことにする。

Router Advatiseの受信設定
-------------------------

ビルド時に `UIP_CONF_ROUTER` が有効だと、
ルーターとして、Router Advatiseを受け付けない。
makeからのオプションでは変更できないので、
ファイルを修正する。

```
diff --git a/platform/jn516x/contiki-conf.h b/platform/jn516x/contiki-conf.h
index e3c3a21d9..5ea73c7dd 100644
--- a/platform/jn516x/contiki-conf.h
+++ b/platform/jn516x/contiki-conf.h
@@ -125,7 +125,7 @@
 #define UIP_CONF_LL_802154              1
 #define UIP_CONF_LLH_LEN                0

-#define UIP_CONF_ROUTER                 1
+#define UIP_CONF_ROUTER                 0
 #ifndef UIP_CONF_IPV6_RPL
 #define UIP_CONF_IPV6_RPL               1
 #endif /* UIP_CONF_IPV6_RPL */
```

udp-client.cの接続先の指定
------------

接続先が決め打ちになっているので、`udp-client.c` を書き換えて、
自前の環境に合わせたアドレスに設定する。

```
diff --git a/examples/udp-ipv6/udp-client.c b/examples/udp-ipv6/udp-client.c
index 85854a48b..26a153666 100644
--- a/examples/udp-ipv6/udp-client.c
+++ b/examples/udp-ipv6/udp-client.c
@@ -113,7 +113,8 @@ set_connection_address(uip_ipaddr_t *ipaddr)
 #elif UIP_CONF_ROUTER
 #define UDP_CONNECTION_ADDR       fd00:0:0:0:0212:7404:0004:0404
 #else
-#define UDP_CONNECTION_ADDR       fe80:0:0:0:6466:6666:6666:6666
+//#define UDP_CONNECTION_ADDR       fe80:0:0:0:6466:6666:6666:6666
+#define UDP_CONNECTION_ADDR       fd80::213:a200:40b6:e2bd
 #endif
 #endif /* !UDP_CONNECTION_ADDR */
```

mDNSを使わない場合は、mDNSを無効にしておく。

```
diff --git a/examples/udp-ipv6/Makefile b/examples/udp-ipv6/Makefile
index 15230c4f0..034702d5e 100644
--- a/examples/udp-ipv6/Makefile
+++ b/examples/udp-ipv6/Makefile
@@ -2,5 +2,5 @@ all: udp-server udp-client

 CONTIKI = ../..
 CONTIKI_WITH_IPV6 = 1
-CFLAGS += -DUIP_CONF_ND6_SEND_NS=1
+CFLAGS += -DUIP_CONF_ND6_SEND_NS=1 -DRESOLV_CONF_SUPPORTS_MDNS=0
 include $(CONTIKI)/Makefile.include
```

mDNSを使う場合は、ホスト側の `/etc/avahi/hosts` に
`contiki-udp-server.local` の設定を追加する。
lowpan0のアドレスを設定する。


```
fe80::213:a200:4090:4064 contiki-udp-server.local
```


最後に、RPLの無効をしてビルドする。

```
make TARGET=jn516x CHIP=JN5164 -j4 CONTIKI_WITH_RPL=0
```

これで、出来たバイナリをTWE-Liteに焼き込むと、


udp-client.c 解題
----------------

ソースの追いかけてContikiでのUDPの使い方を見てみる。

まず、`AUTOSTART_PROCESS` に書かれているのが、起動される処理になる。

```
AUTOSTART_PROCESSES(&resolv_process,&udp_client_process);
```

`resolv_process` は DNSによる名前解決の機能を起動している。
これはContikiOSが持っているOSの機能になる。
`udp_client_process` がこのプログラムのメインの処理になる。


メインの処理の冒頭は送信先のアドレスを決めている。
必要であればDNSに問い合わせて名前解決を行う。

```
  static resolv_status_t status = RESOLV_STATUS_UNCACHED;
  while(status != RESOLV_STATUS_CACHED) {
    status = set_connection_address(&ipaddr);

    if(status == RESOLV_STATUS_RESOLVING) {
      PROCESS_WAIT_EVENT_UNTIL(ev == resolv_event_found);
    } else if(status != RESOLV_STATUS_CACHED) {
      PRINTF("Can't get connection address.\n");
      PROCESS_YIELD();
    }
  }
```

名前解決に失敗したら、`PROCESS_YIELD`が呼ばれ、

UDPは、`udp_new` と `udp_bind` で初期化している。
Linuxでのsocketの処理と似たような構造になっている。

```
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));
```
udp の初期化が終わったら、周期実行を行う無限ループに入る。

```
  etimer_set(&et, SEND_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }
```

タイムアウトか、tcpipのイベントが発生したら、
`timeout_handler` を呼び出している。
`timeout_handler` では実際のデータ送信を行う。

```
  PRINT6ADDR(&client_conn->ripaddr);
  sprintf(buf, "Hello %d from the client", ++seq_id);
  printf(" (msg: %s)\n", buf);
#if SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION
  uip_udp_packet_send(client_conn, buf, UIP_APPDATA_SIZE);
#else /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
  uip_udp_packet_send(client_conn, buf, strlen(buf));
#endif /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
```
`uip_udp_packet_send` で UDPパケットの送信を行う。


//raw[|latex| \\clearpage ]

hoge
