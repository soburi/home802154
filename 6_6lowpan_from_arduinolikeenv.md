TWE-Lite/Arduino互換環境 から 6LoWPANする
=======================

//raw[|latex| \\thispagestyle{empty} ]


TWE-Lite向けArduino互換環境
---------------------------

TWE-Lite向けにArduinoの開発環境、
APIで開発が行える環境を作成したので、
手前味噌ながら、こちらを使って6LoWPANの通信を行ってみる。
Arduino互換となるので、開発に必要な予備知識は
Contikiのプログラム開発と比べると相当に少なくて済む。
またAPIとしては、Arduinoの`Ethernet`のライブラリと
ほぼ同一となっているので、こちらを使ったことがあれば
スムーズに入れるはずだ。


udp-echo-client
----------------

こちらも簡単なサンプルとして、echoクライアントのUDP版を取り上げる。

contikiのサンプルとほぼ同じ動きだが、
well known portの7番ポートで通信していることと、
送信先に指定しているホスト名が違うところになる。



```
#include <MicroIp.h>
#include <MicroIpUdp.h>
```

まず、includeは`Ethernet.h`の代わりに`MicroIp.h`となる。
ContikiのIP実装はuIPと呼ばれている。

```
void setup() {
  MicroIP.begin();
  server = MicroIP.lookup("udp-echo-server.local");

  Udp.begin(localPort);
}
```

`MicroIP.begin`で初期化処理を行ったのち、
`MicroIP.lookup`で送信先の名前を解決する。
この`MicroIP.lookup`の戻り値はIPAddress型のクラスインスタンスになるが、
これもIPv6の仕様にあわせて拡張している。
最後に`Udp.begin`でlistenポートを設定している。
これはLinuxでSocketを使う時の`bind`と同様の操作となる。

`loop()`の中では、データの送信、受信を行っている。
送信はシリアルから入力されたデータがあれば、
バッファに詰めて送信している。

```
  Udp.beginPacket(server, localPort); 
  Udp.write(packetBuffer, len);
  Udp.endPacket();
```

これもArduino標準の`UDP`と互換の動作となっている。
`Udp.beginPacket`で送信用のバッファリングを開始、
`Udp.write`でバッファへの書き込み、`Udp.endPacket`
でバッファの内容を確定、送信を行う。

```
  while(int packetSize = Udp.parsePacket()) {
    IPAddress remote = Udp.remoteIP();
    Serial.print(remote);
    Serial.println(Udp.remotePort());
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    for(int i=0; i<packetSize; i++) {
      Serial.print(packetBuffer[i]);
    }
  }
```


ここで、`Udp.parsePacket()`は受信したパケットをバッファに展開し、
データサイズを返す。受信したパケットがなければ0を返す。
次の`Udp.parsePacket()`の呼び出しまでは、
Udp.remoteIP(), Udp.remotePort(),Udp.read()は
ここで展開したパケットの情報を返す。

ここでは単純に受信データの表示のみ行っているので、
展開したパケットの情報を表示している。

ArduinoのUDPの実装はもともとWIZnet W5500という、
ハードウェアでIPのプロトコルスタックを持ったチップを利用していた。
そのため、`Udp.parsePacket()`のような状態依存の挙動がある、
若干癖のあるAPIとなっている。


