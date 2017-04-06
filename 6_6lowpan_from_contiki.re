
= TWE-Lite/Contiki から 6LoWPANする


//raw[|latex| \thispagestyle{empty} ]



TWE-Liteを使って、ContikiOSの6LoWPANの通信を行う。



contikiのサンプルでは @<tt>{examples/udp-ipv6} が通信確認に
向いたシンプルな通信プログラムになっている。
これを使って、Linux側との通信を行う。


== RPL, NDP


IEEE802.15.4などの省電力ネットワーク向けにRPL
(IPv6 Routing Protocol for Low-Power and Lossy Networks)
というルーティングプロトコルがあり、Contikiもこれの実装を持っている。
が、Linux側に十分こなれた実装がない。
今回は普通のv6ネットワークで使う、
NDP(Neighbor Discovery for IP version 6 (IPv6))で
Router Advatiseをradvdから通知してルーター情報の設定を行う。


== Router Advatiseの受信設定


ビルド時に @<tt>{UIP_CONF_ROUTER} が有効だと、
ルーターとして、Router Advatiseを受け付けない。
makeからのオプションでは変更できないので、
ファイルを修正する。


//emlist{
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
//}

== 接続先の指定


接続先が決め打ちになっているので、@<tt>{udp-client.c} を書き換えて、
自前の環境に合わせたアドレスに設定する。


//emlist{
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
//}


mDNSによる解決も動作が怪しいので無効化しておく。


//emlist{
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
//}


最後に、RPLの無効をしてビルドする。


//emlist{
make TARGET=jn516x CHIP=JN5164 -j4 CONTIKI_WITH_RPL=0
//}


これで、出来たバイナリをTWE-Liteに焼き込むと、


== ContikiのUDP API


UDPのオープンは、


//emlist{
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));
//}


実際にデータを投げるのは


//emlist{
  uip_udp_packet_send(client_conn, buf, strlen(buf));
//}
