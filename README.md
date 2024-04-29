# Merimote v0.0.1  
Merimote is a module for receiving remote control signals.  
  
メリモートは主にM5StampPICOを用いたマルチユースのリモコン受信デバイスです。  
オープンソースのため拡張しやすく、好みのリモコン環境を構築できます。  
Meridianシステムの一部ですが、Merimote単独でも利用することができます。  
  
## 現在の対応デバイスと出力  
・M5StampPICO  
・I2C出力  
・Wiiリモコン受信  
・PS4受信  
  
## M5StampPICOである理由  
多くのリモコンに対応するためには、SPPが必要があります。  
M5StampPICOは小型でSPPに対応したデバイスです。  
  
## 受信機をマイコンと分けたい理由  
ESP32などで高速なWifi通信を行う場合、Bluetoothと併用することでリアルタイム処理が間に合わなくなる場合があります。  
そこでリモコンの受信処理を分散し、整理したデータをI2Cで渡すようにしました。  
また別モジュール化して疎結合化したことで、個別に拡張しやすくなるという利点があります。  
  
## I2Cデータフォーマット  
デフォルトのI2Cアドレスは0x58です。  
呼び出すことで下記の配列でデータを返します。  

|byte|0,1|2|3|4|5|6|7|8,9|  
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|  
|byte|ボタンデータ|Stick_L_x|Stick_L_y|Stick_R_x|Stick_R_y|L2アナログ|R2アナログ|checksum|  

Short型で[0]~[4],Byte型で[0]~[9]となる共用体配列です。  
チェックサムはshort型で[0]~[3]まで合計したものをビット反転したものになります。  

その他の仕様については[Meridian](https://ninagawa123.github.io/Meridian_info/#aboutMeridim/remort_controller/)に準拠します。
