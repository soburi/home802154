
= IEEE802.15.4 Network Programming


//raw[|latex| \thispagestyle{empty} ]


== 通信の方法


LinuxでIEEE802.15.4を使った通信を行うためには、大きく2つの方法がある。
一つは、Socketをつかって、ほぼ直接IEEE802.15.4のデータを送受信する方法。
もう一つは、6LoWPANを上位層として、IPv6として通信する方法だ。


== Socketの通信


Socket, 6LoWPANいずれも、シンプルなサンプルプログラムが
wpan-tools に含まれている。



@<href>{https://github.com/linux-wpan/wpan-tools/tree/master/examples,https://github.com/linux-wpan/wpan-tools/tree/master/examples}



examples の @<tt>{af_ieee802154_tx.c}, @<tt>{af_ieee802154_rx.c}が
Socketによる通信のサンプルとなっている。



まず、IEEE802.15.4のアドレス定義は
/usr/include/sysの配下には入っていないので、
カーネルに含まれる以下の定義をコピーしてソースに組み込む
必要がある。


//emlist{
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
//}


改めて、IEEE802.15.4のアドレスの定義を確認しよう。



IEEE802.15.4にはShort AddressとLong Address の2つの方法でアドレスを指定できる。
いずれを使うかもアドレスの情報に含まれる。
Short address は PAN内 でのみユニークなIDで16bitの数値である。
0xFFFFと0xFFFEはそれぞれ、ブロードキャストと未割当の予約値で、これ以外の値が
有効なアドレスとなる。
Long Addressは64bitの 拡張MACアドレス(EUI-64)のハードウェアアドレスである。
これらのいずれかのアドレスに加えて、所属するPANを示すためのPAN IDを持つ。



物理アドレスの性格を持つLong Addressと論理アドレスとしてのShort Address
の両方を含めるアドレス表現を定義している。 
アドレスの定義さえIEEE802.15.4の定義に従うものにしてしまえば、
その先は至って普通のソケット通信である。



@<tt>{af_ieee802154_tx.c}では、まず送信先のアドレスとバッファを設定する。
意味的には以下のコードと同じ操作。
@<tt>{pan_id}と@<tt>{hwaddr}は使っている環境に合わせて書き換える必要がある。


//emlist{
struct sockaddr_ieee802154 dst = {
    .family = AF_IEEE802154,
    .addr = {
        .addr_type = IEEE802154_ADDR_LONG,
        .pan_id = 0xabcd,
        .hwaddr =    {0x00, 0x13, 0xa2, 0x00,
                  0x40, 0xa7, 0x5e, 0xcc},
    }
};

unsigned char buf[MAX_PACKET_LEN + 1] =
           "Hello world from IEEE 802.15.4 socket example!";
//}


ソケットを作成して、 sendtoで送信する。


//emlist{
sd = socket(PF_IEEE802154, SOCK_DGRAM, 0);
len = sendto(sd, buf, strlen(buf), 0, (struct sockaddr *)&dst, sizeof(dst));
//}


受信側の@<tt>{af_ieee802154_rx.c}も同様にソケットを作成して@<tt>{bind}したのち、
@<tt>{recvfrom}で受信する。
@<tt>{bind}はIEEE802.15.4のインターフェースを指定するため、
インターフェースの持つアドレスを指定してsocketを括りつけるための操作。


//emlist{
sd = socket(PF_IEEE802154, SOCK_DGRAM, 0);
ret = bind(sd, (struct sockaddr *)&src, sizeof(src));
while(1) {
    ret = recvfrom(sd, buf, MAX_PACKET_LEN, 0, (struct sockaddr *)&dst, &addrlen);
    ...
}
//}


アドレスの定義と、最大データ長にさえ気を付ければ、
socketを使った(UDPとほぼ同様の)データグラム通信として扱える。


== 6LoWPANの通信


6LoWPANの通信サンプルも、wpan-toolsに含まれている。
とはいえ、6LoWPANはIPv6からほぼ透過に見える実装となるため、
実際問題としては、ただのUDP/IPv6のプログラムだ。



wpan-toolsのサンプルプログラムでは、
socketを作成したのち、
ブロードキャスト先を限定するため
@<tt>{SO_BINDTODEVICE} を指定している。これは、サンプルの
通信が確実にIEEE802.15.4のデバイス経由で行われるようにするための措置。


//emlist{
sd = socket(PF_INET6, SOCK_DGRAM, 0);
ret = setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr));
//}


その後はシンプルに@<tt>{sendto}/@<tt>{recvfrom} でデータグラム通信を行っている。


//emlist{
ret = sendto(sd, buf, strlen(buf), 0, (struct sockaddr *)&dst, sizeof(dst));
//}

//emlist{
len = recvfrom(sd, buf, MAX_PACKET_LEN, 0, (struct sockaddr *)&dst, &addrlen);
//}


IEEE802.15.4経由で6LoWPANを使う場合に気を付けるのは、
IEEE802.15.4のアドレスとIPv6アドレスとの対応づけだ。
EUI64からは以下の手順でIPv6のInterfaceIDが算出される。

 * 先頭アドレスから7ビット目のビットを反転させる。



実例として、 @<tt>{00-13-a2-00-40-b6-e2-bd} のEUI64アドレスに対しては、
先頭から7bit目を反転して InterfaceIDの @<tt>{0213:a200:40b6:e2bd} が求められる。
これに Link Local のプリフィクス @<tt>{fe80:0:0:0} がついて、
v6アドレスは


//emlist{
fe80:0:0:0:213:a200:40b6:e2bd
//}


となる。



Short Addressからは以下の手順でInterfaceIDを求める。

 * PAN_ID と ショートアドレスの間に @<tt>{0x00fffe00} を挟んで 64ビットの値を作る
 * 先頭アドレスから7ビット目のビットを反転させる。



PANID:@<tt>{0x3332} ショートアドレス:@<tt>{0x4321} のアドレスからは
間に@<tt>{0x00fffe00} の16ビットを挟んで、
@<tt>{33-32-00-ff-fe-00-43-21}の64bit値を作る。
これの7bit目を反転して @<tt>{3132:00ff:fe00:4321} がInterfaceIDとなる。
Link Localをつけて


//emlist{
fe80::3232:ff:fe00:4321 
//}


がIPv6のアドレスとなる。

