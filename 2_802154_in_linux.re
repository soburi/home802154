
= LinuxでのIEEE802.154/6LoWPAN


//raw[|latex| \thispagestyle{empty} ]


== 前史


LinuxのIEEE802.15.4の実装は2008年ごろに
linux-zigbeeプロジェクトとして始まっている。
Siemensらが主導していた。
(Linux/documentations/の記載もこの時期のものである)



2012年にメインラインにマージが行われるが、
ZigbeeのライセンスのOSSとの親和性のなさもあって、
このころには活動はかなり低調となっている。



この時期の実装では、IEEE802.15.4/ZigBeeの枠組みで
設計、実装が行われているのが特徴となる。
ユーザーランドのツールの @<tt>{iz}, @<tt>{izattach} という
名前にその名残を見ることができる。
(lowpan-toolsはDebian/jessieまで収録されている)


== linux-wpan


その後新たにlinux-wpanプロジェクトがIEEE802.15.4の
実装を主導している。
ZigBeeの名前はすでに外され、6LoWPANを介して
IPネットワーク(=Internet)との相互運用を行う実装が進められている。



2014年にIEEE802.15.4の実装から6LoWPANの共通部分が
切り出された。6LoWPANはBluetoothを下位層とする
実装もあり、上位層を共通部として分離した。



ドライバ層も、2014年に共通実装部分の整理が行われており、
net namespace への対応の共通化などが行われている。


== LinuxでのIEEE802.15.4の使い方


LinuxでIEEE802.15.4で通信するためには2つの方法がある。



1つは6LoWPANを用いる方法で、6LoWPAN(=IPv6)のデバイスとして、
v6アドレスを割り当てて通信する方法である。
6LoWPANの層でパケットの分割が行われるにせよ、
TCPのストリーム的にはほぼ透過的に扱える。



このため、 IPv6が扱えるプログラムで 
@<tt>{一応IEEE802.15.4経由のTCP通信が扱える。}



UDPについては、6LoWPANの層でパケットの分割が行われるため、
それを意識した通信の実装に修正する必要がある。



もう一つはデータグラム通信を行うソケットとして扱う方法。
これは、IEEE802.15.4向けに @<tt>{AF_IEEE802154} が定義されているので、
これを使って、socketの @<tt>{send/recv}, @<tt>{sendto/recvfrom} を使って
通信ができる。
