<img width="400" alt="merimote" src="https://github.com/user-attachments/assets/562b162e-1b4a-4766-8667-0760f6f7ee3f">

# [Merimote v0.0.1](https://github.com/Ninagawa123/Merimote)  
Merimoteは主にM5StampPICOを用いたマルチユースのリモコン受信デバイスです。  
Arduino frameworkのオープンソースのため拡張しやすく、好みのリモコン環境を構築できます。  
Meridianシステムの一部ですが、もちろんMerimote単独でも利用することができます。  
M5StampPICOとArduinoUNOの接続サンプルを用意しましたので、I2Cの動作確認もすぐに試せます。  
Merimote is a module for receiving remote control signals.  
Merimote primarily uses the M5StampPICO as a multi-use remote control receiving device. It is open source, making it easy to extend and allows for building a custom remote control environment. It is part of the Meridian system, but can also be used independently.  
I have prepared a connection sample for the M5Stamp PICO and Arduino UNO, so you can also immediately test the I2C functionality.  
  
<br>
## Current Compatible Devices and Outputs  

・M5StampPICO  
・I2C output  
・Wii Remort
・PS4 Controler  
  
<br>
## Why M5StampPICO  

多くのリモコンに対応するためには、SPPが必要があります。  
M5StampPICOは小型でSPPに対応したデバイスです。  
To support many remotes, SPP is necessary. M5StampPICO is a compact device that supports SPP.  
  
<br>
## Reasons for Separating the Receiver from the Microcontroller

ESP32などで高速なWifi通信を行う場合、Bluetoothと併用することでリアルタイム処理が間に合わなくなる場合があります。  
そこでリモコンの受信処理を分散し、整理したデータをI2Cで渡すようにしました。  
また別モジュール化して疎結合化したことで、個別に拡張しやすくなるという利点があります。  
When performing high-speed Wifi communications with devices like the ESP32, combining it with Bluetooth can delay real-time processing. Therefore, remote control reception is decentralized, and organized data is transmitted via I2C. Also, modularizing and decoupling facilitate easy individual extensions.  
  
<br>
## I2C Data Format  

デフォルトのM5StampPICOのI2Cアドレスを0x58と設定しています。  
呼び出すことで下記の配列データを返します。  
The default I2C address for the M5StampPICO is set to 0x58. It returns the following array data when called:

|byte index|0,1|2|3|4|5|6|7|8,9|  
|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|  
|short index|0|1_high|1_low|2_high|2_low|3_high|3_low|4|  
|Assign|Button|Stick_L_x|Stick_L_y|Stick_R_x|Stick_R_y|L2_analog|R2_analog|checksum|  
|Type|BIN|-127to127|-127to127|-127to127|-127to127|0to255|0to255|HEX|  

Short型でindex0〜4、Byte型でindex0〜9となる共用体配列です。  
チェックサムはshort型のindex0~3までを合計後にビット反転したものになります。  
その他の仕様については[Meridian](https://ninagawa123.github.io/Meridian_info/#aboutMeridim/remort_controller/)に準拠します。  
This is a union array with Short type ranging from index 0 to 4 and Byte type from index 0 to 9. The checksum is calculated by summing up Short type from index 0 to 3 and then inverting the bits. For further specifications, please refer to Meridian, which this complies with.  

<br>
## How To Install  

PlatformIOでMerimote>MerimotePICOフォルダを開き、M5StackPICOにインストールします。  
通信先となるマイコンボードとSCK同士、SDA同士、GND、5Vを接続します。  

<br>
## How To Use  

config.hで設定を行います。  

```
#define MOUNT_JOYPAD 4 // 接続するジョイパッドの種類です。現在は4:PS4 ,5:Wii_yoko が使えます。  
#define I2C_DEV_ADDR 0x58 // I2Cデバイスアドレスを設定できます。  
#define MONITOR_JOYPAD 1 // 動作確認用にシリアルモニタで入力データを表示できます。（0:OFF, 1:ボタンのみ, 2:アナログ込み） 
```

<br>
## LED  

PIN25(DAC)とGNDの間に抵抗入りLEDを接続すると、リモコンペアリング前は点滅し、ペアリング後に常時点灯となります。  
  
<br>
## Reset Button  

PIN32と5Vをショートするとソフトウェアリセットし、再度ペアリングができるようになります。  
  
<br>
## Trouble Shooting   

接続が確立しない場合、  

```
#define BT_REMOVE_BONDED_DEVICES 1  
```

を書き込んで再起動すると、BTデバイスの登録がリセットされ、接続できるようになるかもしれません。  
  
<br>
## Wiiリモコンの接続方法  

```
#define MOUNT_JOYPAD 5 
```

として書き込みます。  
Wiiリモコンの1,2ボタンを同時押しするとペアリングします。  
  
<br>
## PS4リモコンの接続方法  

```
#define MOUNT_JOYPAD 4 
```

として書き込みます。  
起動するとシリアルモニタにM5StampPICOのMACアドレスがxx:xx:xx:xx:xx:xxのフォーマットで表示されるのでメモします。  
**SixaxisPairTool**を検索してPCにダウンロードし、SixaxisPairToolの起動後にPS4リモコンをUSBケーブルでPCに接続します。  
接続先のMacアドレスを設定できるようになるので、メモしたアドレスを上書き登録します。  
M5StampPICOの起動後にPSボタンを押すと、ペアリングが確立します。  

<br>
## 動作確認  
簡易的に動作確認ができるArduino UNO用のサンプルスクリプトを用意しました。  
Arduino UnoなどとM5StampPICOをSCK同士、SDA同士、V5、GNDをそれぞれ結線し、Sample＞Merimote_Receiver.inoをArduino IDE等で書き込んでください。  
実行後にシリアルモニタで下図のように動作確認ができます。  

<img width="400" alt="Merimote_Receiver" src="https://github.com/Ninagawa123/Merimote/assets/8329123/2bd1d101-ddce-4497-9dcd-9cdfd1a659a0">
  
## BlueRetroを使えばよいのでは？
MeridianはBlueRetro準拠に移行予定です。  
一方、Merimoteはコードベースで内容のカスタマイズができて便利な面も多いので、必要に応じて育てていこうと思います。  
