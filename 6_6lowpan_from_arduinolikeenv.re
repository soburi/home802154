
= TWE-Lite/Arduino互換環境 から 6LoWPANする


//raw[|latex| \thispagestyle{empty} ]


== TWE-Lite向けArduino互換環境


TWE-Lite向けにArduinoの開発環境、
APIで開発が行える環境を作成したので、
手前味噌ながら、こちらを使って6LoWPANの通信を行ってみる。
Arduino互換となるので、開発に必要な予備知識は
Contikiのプログラム開発と比べると相当に少なくて済む。
またAPIとしては、Arduinoの@<tt>{Ethernet}のライブラリと
ほぼ同一となっているので、こちらを使ったことがあれば
スムーズに入れるはずだ。



TWE-Lite向けArduino互換環境はArduinoのボードマネージャからインストールできる
パッケージとして配布している。
Arduinoの環境設定からURLを指定する。
@<tt>{ファイル} → @<tt>{環境設定} のダイアログの、
@<tt>{追加のボードマネージャのURL}の欄に、以下のURLを追加する。


//emlist{
http://soburi.github.io/JN516x-arduino-package/package_soburi_intiki_index.json
//}


設定を行い、@<tt>{ツール} → @<tt>{ボード:...} → @<tt>{ボードマネージャ...} を
選択すると、ダイアログのリストの下の方に @<tt>{JN516x Boards} の項目が現れる。
この項目を選択し、インストールを実行する。


== udp-echo-client


こちらも簡単なサンプルとして、echoクライアントのUDP版を取り上げる。



contikiのサンプルとほぼ同じ動きだが、
well known portの7番ポートで通信していることと、
送信先に指定しているホスト名が違うところになる。


//emlist{
#include <MicroIp.h>
#include <MicroIpUdp.h>
//}


まず、includeは@<tt>{Ethernet.h}の代わりに@<tt>{MicroIp.h}となる。
ContikiのIP実装はuIPと呼ばれている。


//emlist{
void setup() {
  MicroIP.begin();
  server = MicroIP.lookup("udp-echo-server.local");

  Udp.begin(localPort);
}
//}


@<tt>{MicroIP.begin}で初期化処理を行ったのち、
@<tt>{MicroIP.lookup}で送信先の名前を解決する。
この@<tt>{MicroIP.lookup}の戻り値はIPAddress型のクラスインスタンスになるが、
これもIPv6の仕様にあわせて拡張している。
最後に@<tt>{Udp.begin}でlistenポートを設定している。
これはLinuxでSocketを使う時の@<tt>{bind}と同様の操作となる。



@<tt>{loop()}の中では、データの送信、受信を行っている。
送信はシリアルから入力されたデータがあれば、
バッファに詰めて送信している。


//emlist{
  Udp.beginPacket(server, localPort); 
  Udp.write(packetBuffer, len);
  Udp.endPacket();
//}


これもArduino標準の@<tt>{UDP}と互換の動作となっている。
@<tt>{Udp.beginPacket}で送信用のバッファリングを開始、
@<tt>{Udp.write}でバッファへの書き込み、@<tt>{Udp.endPacket}
でバッファの内容を確定、送信を行う。


//emlist{
  while(int packetSize = Udp.parsePacket()) {
    IPAddress remote = Udp.remoteIP();
    Serial.print(remote);
    Serial.println(Udp.remotePort());
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    for(int i=0; i<packetSize; i++) {
      Serial.print(packetBuffer[i]);
    }
  }
//}


ここで、@<tt>{Udp.parsePacket()}は受信したパケットをバッファに展開し、
データサイズを返す。受信したパケットがなければ0を返す。
次の@<tt>{Udp.parsePacket()}の呼び出しまでは、
Udp.remoteIP(), Udp.remotePort(),Udp.read()は
ここで展開したパケットの情報を返す。



ここでは単純に受信データの表示のみ行っているので、
展開したパケットの情報を表示している。



ArduinoのUDPの実装はもともとWIZnet W5500という、
ハードウェアでIPのプロトコルスタックを持ったチップを利用していた。
そのため、@<tt>{Udp.parsePacket()}のような状態依存の挙動がある、
若干癖のあるAPIとなっている。

