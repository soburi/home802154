

tcp-echo-client
---------------

もう一つ、ArduinoのTCP通信の互換APIを使った
echoクライアントの例を見ていく。

省電力ネットワークはその目的から、持続的な接続となる
ストリーム型の通信とは相性が良くない。
その観点で見れば6LoWPAN上のTCP通信は蛇足みたいなところはあるが、
既存の資産をそのまま流用可能というところにおいては、
実用的な意味があるはずである。(httpクライアントとかね)
ここでは、有用性にはこだわらず「あるに越したことはない」という
ところから、この機能を使ってみることにする。

TCPのクライアントは`MicroIPClient`を使う。
これも、Arduinoの`EthernetClient`とほぼ互換となる
APIである。

```
MicroIPClient client;
```

接続されていなければ、接続先のホスト名とポートを指定して
接続する。ホスト名はIPAddressでも文字列でも受け付ける。
文字列の場合は内部で`lookup`を行って解決する。

```
  while( !client.connected() ) {
    int ret = client.connect("echo-server.local", 7);
    Serial.print("connect ");
    Serial.println(ret);
    delay(1000);
  }
```

`MicroIPClient`は、ArduinoのSerialと同じ、
`Stream` の共通操作で読み書きができる。

`available`で受信できているバイト数を確認して
`read`で読みだす。

```
  while(client.available() ) {
    buf[count] = client.read();
    ...
```

送信データとして、
シリアルから読んだデータをバッファに格納して
`write`で送信する。
`write`の呼び出し単位で送信が行われるので、
送信単位ごとにバッファに詰める。

```
  client.write(buf, count);
```

