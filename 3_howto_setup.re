
= IEEE802.15.4ネットワークのセットアップ


//raw[|latex| \thispagestyle{empty} ]


== 必要な機材の準備(ホスト側)
 * RaspberryPi (Raspbianがインストールされているもの)
 * XBee/XBee Pro S1 もしくは XBee S2C
 * XBee 接続用シリアルアダプタ(以下アダプタ)



通信の確認を行う場合は上記が2セット必要となる。


== セットアップ

=== XBeeカーネルドライバのコンパイル


@<href>{https://github.com/soburi/XBee-linux-serial-driver,https://github.com/soburi/XBee-linux-serial-driver}
をコンパイルする。


//emlist{
make
sudo make install
sudo depmod
//}


を実行する。


== コマンドラインによる設定

=== モジュールのロード


IEEE802.15.4のカーネルモジュールをロードする。
modprobeで @<tt>{ieee802154}, @<tt>{mac802154}をロードする。


//emlist{
/sbin/modprobe ieee802154
/sbin/modprobe mac802154
//}


Socketを使った通信を行う場合はさらに@<tt>{ieee802154_socket}を、


//emlist{
/sbin/modprobe ieee802154_socket
//}


6LoWPANを使う場合は、@<tt>{ieee802154_6lowpan}も


//emlist{
/sbin/modprobe ieee802154_6lowpan
//}


ロードする。



XBee用のデバイスドライバは


//emlist{
/sbin/modprobe xbee802154
//}


で読み込む。


=== XBeeの接続


RaspberryPi の USBポートにアダプタを接続し、
XBeeを接続する。


=== ldattach


XBeeはシリアルで接続されているため、
接続先がXBeeであるかは自動的に検知できない。
そのため、シリアルの制御方式を明示的に
通知する操作が必要となる。
この操作は@<tt>{ldattach}コマンドによって行われ、
シリアルポートの設定と制御方式(つまりドライバの種類)を
カーネルに通知する。



XBeeドライバはカーネル内に種別の定義がないので、
明示的な定義のない25を使っている。



@<tt>{ldattach}を行うと、XBeeドライバでネットワークインターフェース
がカーネルに登録される。


=== Socket, 6LoWPANのインターフェースの立ち上げ


@<tt>{ldattach}で登録されたネットワークインターフェースを
立ち上げる。



Socketを使用する場合は、wpan0をupする。


//emlist{
ip link set wpan0 up
//}


6LoWPANの場合は、IEEE802.15.4のデバイスの上に
lowpanの層を被せたlowpan0を作成し、それをupする。


//emlist{
ip link add link wpan0 name lowpan0 type lowpan
ip link set lowpan0 up
//}


これで、ユーザーランドのプログラムからの通信が可能となる。


== 起動時設定ファイルの作成

=== XBeeの起動処理の流れ


立ち上げるコマンドは単純だが、
立ち上げの契機をDebianの仕組みに則ってやると、
設定ファイルが分散して、少々面倒である。
順に追っていく。

 * システム起動
 ** モジュールのロード
 * USB接続検知
 ** ldattach実行
 * ネットワーク検知
 ** interface立ち上げ
 *** radvd再起動


=== /etc/modules


システムの起動時に @<tt>{/etc/modules} を参照して、
モジュールをロードする。
IEEE802.15.4のモジュールを起動時にロードするには、
@<tt>{/etc/modules} に 単純に以下の行を追加する。


//emlist{
ieee802154
mac802154
ieee802154_socket
ieee802154_6lowpan
xbee802154
//}

=== /etc/udev/rules.d/99-xbee802154.rule


XBeeがつながったアダプタの接続検知で、
ldattachを実行する。



USBデバイスの接続検知はudevで行うが、
アダプタにはFT232 が使われてる場合が多いので、
他のFT232のデバイスが接続されたときに識別できるようにする必要がある。



@<tt>{FT_Prog}@<fn>{1} のユーティリティでFT232のパラメータを設定して、デバイスを認識、
ドライバのロードを自動化することができる。



USBのVID,PIDを変更するのはusb.orgの管理なので仕様上は独自の番号がつけられない。
そもそもFT232の機能が変わるわけでもないのでちょっと大げさである。
SerialNumberだと、同種のデバイスが複数あるときにやりづらい。



Product Descriptionの文字列で識別するのが簡単である。



usb-serialが読み込まれたときに、
以下のような感じで、VID,PIDはFT232のものを利用して、
Productの文字列がマッチした場合のみに
systemdに登録したldattachを実行する。


//emlist{
ACTION=="add", SUBSYSTEM=="tty", ENV{ID_MODEL}=="XBee_UART_Adapter", \
  ENV{ID_VENDOR_ID}=="0403", ENV{ID_MODEL_ID}=="6001", \
  RUN+="/bin/sh -c '/bin/systemctl start ldattach-xbee@$name'"
ACTION=="remove", SUBSYSTEM=="tty", ENV{ID_MODEL}=="XBee_UART_Adapter", \
  ENV{ID_VENDOR_ID}=="0403", ENV{ID_MODEL_ID}=="6001", \
  RUN+="/bin/sh -c '/bin/systemctl stop ldattach-xbee@$name'"
//}

=== /lib/systemd/system/ldattach-xbee@.service


@<tt>{systemd}にl@<tt>{dattach}の起動設定を追加する。
パラメータに@<tt>{/dev/} 以下のtty名を取るようにする。
ldattachは普通に起動するとdaemon化するので、
@<tt>{-d}をつけてバックグラウンド実行しないようにする。


//emlist{
[Unit]
Description=XBee@/dev/%I

[Service]
Type=simple
ExecStart=/usr/sbin/ldattach -d -s 9600 -8 -n -1 25 /dev/%I
//}

=== /etc/network/interfaces


ldattachでアダプタとドライバが関連付けられると、ネットワークインターフェースが
OSに認識される。
wpanのネットワークインターフェースが認識されたことを契機に、
lowpan0の設定を行い、さらに
lowpan0が認識されたところで radvdの再起動を実行する。


//emlist{
allow-hotplug lowpan0 wpan0

iface wpan0 inet6 auto 
    pre-up /bin/ip link add link wpan0 name lowpan0 type lowpan
    pre-down /bin/ip link delete lowpan0 type lowpan

iface lowpan0 inet6 auto
    post-up systemctl restart avahi-daemon
    post-up systemctl restart radvd

//}

=== /etc/radvd.conf


本稿の記事では、Router Advertiseでルーター情報を配布する。
lowpan0に向けて、プレフィックスを設定する。
/etc/radvd.confにlowpan0に向けた設定を追加して、
radvdを再起動する。


//emlist{
interface lowpan0 {    
          AdvSendAdvert on;
          prefix fd00:0:0:1::/64
          {
          };
};
//}

== iwpan


@<tt>{iwpan}はLinuxのIEEE802.15.4のデバイスの設定を表示・変更するための
コマンドラインのツールである。



Debianでは@<tt>{wpan-tools}のパッケージに含まれている。



LinuxのIEEE802.15.4のデバイスドライバは、物理層とMAC層の2つの
パートで構成されており、iwpanのコマンドもその設計に従ったものになる。



すなわち、@<tt>{iwpan}のサブコマンドで大きく２つ、@<tt>{phy} と @<tt>{dev} があり、
それぞれ、物理層、MAC層の設定を扱う。



物理層では、使用するチャンネルの設定や、電波の出力などを設定する。
MAC層では、デバイスのアドレス、PAN IDなどを設定する。


//footnote[1][@<href>{http://www.ftdichip.com/Support/Utilities.htm#FT_PROG}]
