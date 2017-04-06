
IEEE802.15.4 ネットワークプログラミング
===================

//raw[|latex| \\thispagestyle{empty} ]


Overview
--------

LinuxでIEEE802.15.4を使った通信を行うためには、大きく2つの方法がある。
一つは、Socketをつかって、ほぼ直接IEEE802.15.4のデータを送受信する方法。
もう一つは、6LoWPANを上位層として、IPv6として通信する方法だ。



Socketの通信
----------------
Socket, 6LoWPANいずれも、シンプルなサンプルプログラムが
wpan-tools に含まれている。

[https://github.com/linux-wpan/wpan-tools/tree/master/examples](https://github.com/linux-wpan/wpan-tools/tree/master/examples)

IEEE802.15.4のアドレス定義は
/usr/include/sysの配下には入っていないので、
カーネルの定義をコピーして持ってくる必要がある。

```
enum {
	IEEE802154_ADDR_NONE = 0x0,
	IEEE802154_ADDR_SHORT = 0x2,
	IEEE802154_ADDR_LONG = 0x3,
};

struct ieee802154_addr_sa {
	int addr_type;
	uint16_t pan_id;
	union {
		uint8_t hwaddr[IEEE802154_ADDR_LEN];
		uint16_t short_addr;
	};
};

struct sockaddr_ieee802154 {
	sa_family_t family;
	struct ieee802154_addr_sa addr;
};
```


改めて、IEEE802.15.4のアドレスの定義を確認しよう。

IEEE802.15.4にはShort AddressとLong Address の2つのアドレス方式がある。
いずれかを使うかもアドレス定義の情報に含まれる。
Short address は PAN内 でのみユニークなIDで、Long Addressは64bitの
拡張MACアドレス(EUI-64)のハードウェアアドレスである。
加えて、所属するPANを示すためのPAN IDを持つ。

物理アドレスの性格を持つLong Addressと論理アドレスとしてのShort Address
の両方を含めるアドレス表現を定義している。

アドレスの定義さえIEEE802.15.4の定義に従うものにしてしまえば、
その先は至って普通のソケット通信である。

ソケットを作成して、

```
sd = socket(PF_IEEE802154, SOCK_DGRAM, 0);
```

sendtoで送信する。

```
len = sendto(sd, buf, strlen(buf), 0, (struct sockaddr *)&dst, sizeof(dst));
```

受ける側も、同様にソケットを作成して、bindでアドレスに割り当てて、

```
ret = bind(sd, (struct sockaddr *)&src, sizeof(src));
```

recvfromで受信する。

```
ret = recvfrom(sd, buf, MAX_PACKET_LEN, 0, (struct sockaddr *)&dst, &addrlen);
```


アドレスの定義と、データ長にさえ気を付ければ、
socketを使った(UDPとほぼ同様の)データグラム通信として
扱える。



6LoWPANの通信
--------------

6LoWPANの通信サンプルも、wpan-toolsに含まれている。
とはいえ、6LoWPANはIPv6からほぼ透過に見える実装となるため、
実際問題としては、ただのUDP/IPv6のプログラムだ。

wpan-toolsのサンプルプログラムでは、ブロードキャスト先を限定するため
`SO_BINDTODEVICE` を指定している。

```
ret = setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr));
```

その後は素直にsend/recv。至ってシンプルである。

```
ret = sendto(sd, buf, strlen(buf), 0, (struct sockaddr *)&dst, sizeof(dst));
```

```
len = recvfrom(sd, buf, MAX_PACKET_LEN, 0, (struct sockaddr *)&dst, &addrlen);
```

IEEE802.15.4経由で6LoWPANを使う場合に気を付けるのは、
ハードウェアアドレスとv6アドレスとのマッピングだ。
EUI64のアドレスからは、先頭から7bit目のビットを反転させることで、
v6のInterfaceIDを生成する

実例として、 0x0013a20040b6e2bd のEUI64アドレスに対しては、
先頭から7bit目を反転して 0x0213a20040b6e2bd となり、
Link Local のプリフィクス fe80:0:0:0 がついて、
v6アドレスは

```
fe80::213:a200:40b6:e2bd
```
となる。


PANID:`0x3332` ショートアドレス:`0x4321` のアドレスからは
間に`0x00fffe00` の16ビットを挟んで、
同様に7bit目の反転を行って `0x313200fffe004321` となる。
Link Localをつけて

```
fe80::3232:ff:fe00:4321 
```

となる。




