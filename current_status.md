
LinuxでのIEEE802.15.4運用の問題点
=============================


デバイスの選択肢
--------

Linuxのソースツリーにも複数のIEEE802.15.4のデバイスドライバが
収録されているが、特に日本においては技適の問題で
使用可能なデバイスに大きく制限がある。

今回作成したXBee向けのドライバは、完成度の面でまだ不足があるものの、
電子部品店で入手容易なデバイスで使えるものなので、この問題の解決の
一助となると期待する。


ルーティング機能
------------------

省電力デバイス向けのルーティングプロトコルとして、
RPLの規格があるが、現状、これの実装が停滞しているようである。

* simpleRPL[^1] Pythonによる実装、2013最終更新
* linux-rpl[^2] カーネルでの実装、2014最終更新
* unstrung[^3] Cによる実装、2016最終更新だが、MLは活発ではなさそう

[^1]: [https://github.com/tcheneau/simpleRPL](https://github.com/tcheneau/simpleRPL)
[^2]: [linux-rpl https://github.com/joaopedrotaveira/linux-rpl](https://github.com/joaopedrotaveira/linux-rpl)
[^3]: [https://github.com/mcr/unstrung](https://github.com/mcr/unstrung)

NDPによるルーティング情報の配布はブロードキャスト依存が多く、
省電力デバイスに向かない作りとなっている。
この辺は開発の前進が期待される領域である。
その他、いくつかの省電力ネットワーク向けの
RFC(RFC6775 NDP最適化など) が未実装となっている。




