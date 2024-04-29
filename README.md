<img width="400" alt="merimote" src="https://github.com/Ninagawa123/Merimote/assets/8329123/bf6307bb-e362-42b1-b21c-a4d0929c9ff5">

# Merimote v0.0.1  
Merimoteは主にM5StampPICOを用いたマルチユースのリモコン受信デバイスです。  
オープンソースのため拡張しやすく、好みのリモコン環境を構築できます。  
Meridianシステムの一部ですが、Merimote単独でも利用することができます。  
Merimote is a module for receiving remote control signals.  
Merimote primarily uses the M5StampPICO as a multi-use remote control receiving device. It is open source, making it easy to extend and allows for building a custom remote control environment. It is part of the Meridian system, but can also be used independently.  
  
## Current Compatible Devices and Outputs  
・M5StampPICO  
・I2C output  
・Wii Remort
・PS4 Controler  
  
## Why M5StampPICO  
多くのリモコンに対応するためには、SPPが必要があります。  
M5StampPICOは小型でSPPに対応したデバイスです。  
To support many remotes, SPP is necessary. M5StampPICO is a compact device that supports SPP.  
  
## Reasons for Separating the Receiver from the Microcontroller  
ESP32などで高速なWifi通信を行う場合、Bluetoothと併用することでリアルタイム処理が間に合わなくなる場合があります。  
そこでリモコンの受信処理を分散し、整理したデータをI2Cで渡すようにしました。  
また別モジュール化して疎結合化したことで、個別に拡張しやすくなるという利点があります。  
When performing high-speed Wifi communications with devices like the ESP32, combining it with Bluetooth can delay real-time processing. Therefore, remote control reception is decentralized, and organized data is transmitted via I2C. Also, modularizing and decoupling facilitate easy individual extensions.  
  
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

## How To Install  
PlatformIOでMerimote>M5StampPICOフォルダを開き、M5StackPICOにインストールします。  
通信先となるマイコンボードとSCK同士、SDA同士、GND、5Vを接続します。  
