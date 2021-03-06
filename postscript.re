
= あとがき


//raw[|latex| \thispagestyle{empty} ]



昨年の技術書典から引っ張ってきたネタで、
今回も6LoWPANの話をお届けいたします。



現状国内において、入手性の良いデバイスを使ってIEEE802.15.4の
ネットワークを構築することが非常に困難でありました。
本稿を執筆するにあたって、まずこの問題の解決を図っております。



それは、現状最も入手性のよいIEEE802.15.4デバイスである
XBeeのIEEE802.15.4のドライバを作成することで、
秋葉原で手に入る程度のデバイスでネットワーク構築を行えるようにする
という対応です。



まだ、安定こそしていませんが、LinuxにてIEEE802.15.4を経由した
6LoWPANの通信の確認が行える程度には完成しております。



というわけで、今回のネタは、どちらかといえば、
「ドライバできたよ！」という報告みたいな感じでもあります。



これに加えて、TWE-LiteをArduino互換環境で6LoWPANを動かす
ところまで来ているので、
@<b>{Arduinoを使ってマイコンで6LoWPAN通信を使う} ことが出来ました。



Arudinoと同程度の簡単さで、IoT的な省電力IPネットワークを使える
ので、アキバで売ってるオモチャを使って色々遊ぶことができるかと思います。



ただ、エッジルーターとしてはまだまだ不完全なところがあるので、
カーネルやIEE802.15.4周辺の開発動向を見ながらキャッチアップはまだ
必要そうです。



@<strong>{Best wish for your happy hacking!}

