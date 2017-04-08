//raw[|latex| \\pagestyle{plainhead} ]

はじめに
=====

//raw[|latex| \\thispagestyle{empty} ]



IEEE802.15.4
---------------

IEEE802.15.4[^1] は無線通信規格の一種で、 LANの規格の大分類の802、
中分類の15はPersonal Area Network
その第4グループという意味付けである。
内容としては、省電力・低速の組み込み機器向けの無線ネットワークを定義している。
電池駆動や、エナジーハーベスト(太陽光や振動から電源を供給する)のような
環境で使える20kbps～250kbps程度の低速のネットワークである。
今風にいうなら、IoT向けのネットワークということになる。

[^1]: <http://standards.ieee.org/getieee802/download/802.15.4-2015.pdf>
6LoWPAN
-------

_6LoWPAN_[^2] は IPv*6* over *L*ow poser *W*ireless *P*ersonal *A*rea *N*etworks
の略で、は IPv6の通信を (元々は)IEEE802.15.4 のネットワークに押し込む
ためのプロトコルである。
秋葉原で入手できるような、比較的入手性の良いマイコンでも
IEEE802.15.4を使うことができるので、ちょっとした電子工作に
応用することも可能(なはず)である。

[^2]: <https://datatracker.ietf.org/wg/6lowpan/documents/>

XBeeについて
------------
XBeeは Digi International社[^3]が販売する、
2mmピッチの端子が特徴的な無線モジュールである。
シリアル通信による制御で各種プロトコルの通信を行うことができる。
現在入手可能なIEEE802.15.4の通信を利用できるモデルは、
XBee/XBee Pro S1 802.15.4[^4], XBee S2C[^5]がある。
秋葉原の電子部品店各店でも取り扱いがあり、入手が容易である。

10年ほど前に、Arduinoで使える無線モジュールとして
話題になったが、利活用の情報としてはその頃から
ほとんどアップデートがない。

[^3]: <https://www.digi.com/>
[^4]: <https://www.digi.com/products/xbee-rf-solutions/embedded-rf-modules-modems/xbee-802-15-4>
[^5]: <https://www.digi.com/support/productdetail?pid=3257>

TWE-Liteについて
-------------------

モノワイヤレス株式会社[^6] から発売されている
NXPのJN5164のOEM品(?)がTWE-Lite[^7]である。
国内の技適を取得しており、国内で **合法的** に問題なく使うことができる。
ソフト観点ではJN5164そのものなので、技術情報はJN516xの情報が利用可能である。

[^6]: <https://mono-wireless.com>
[^7]: <https://mono-wireless.com/jp/products/TWE-LITE/index.html>


Contikiについて
---------------
Contiki[^8] は8ビットCPUや、組み込み向けのOSである。
6LoWPANをはじめとして、いわゆるIoT向けの通信機能が
実装されている。

[^8]: <http://www.contiki-os.org/>

本稿について
--------------

本稿ではXBeeを使って、IEEE802.15.4/6LoWPANのネットワークとをIPv6のネットワークを
接続できるエッジルーターの設定方法を紹介する。

本稿を執筆するにあたって、
新たにLinuxのIEEE802.15.4実装が利用できる
XBee用のカーネルドライバを実装した。
本稿ではこれを用いてXBeeでIEEE802.15.4/6LoWPANネットワークの設定方法と、
TWE-Lite/Contikiを使った、ノード側のプログラミング手法について解説する。

本書では、ホスト側の通信モジュールとしてDigi社のXbee S1,
モノワイヤレス社のTWE-Liteを使用する。
