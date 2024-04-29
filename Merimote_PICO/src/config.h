//================================================================================================================
//---- M5 Stamp PICO の 配 線  ----------------------------------------------------------------------------------------
//================================================================================================================
/*
  M5StampPICO :  Main Board
  [GND]      ->  GND
  [5V]       ->  5V
  [21] SDA   ->  SDA
  [22] SCL   ->  SCL
*/

/* 重要な設定 */
#define MOUNT_JOYPAD 5 // 搭載するジョイパッドの種類
                       // 0:なし, 1,2:欠番, 3:PS3(未), 4:PS4 ,5:Wii_yoko, 6:Wii+Nun(未), 7:WiiPRO(未), 8:Xbox(未)

#define I2C_DEV_ADDR 0x58 // このデバイスのI2Cデバイスアドレス

/* シリアルモニタリング */
#define MONITOR_JOYPAD 1 // シリアルモニタでリモコンのデータを表示（0:OFF, 1:ボタンのみ, 2:アナログ込み）
#define SERIAL_PC_BPS 115200 // 115200 // ESP-PC間のシリアル速度（モニタリング表示用）

/* リモコンの設定 */
#define PAD_RECEIVE_RATE 8         // ジョイパッドのBluetooth受信更新頻度（単位ms）
#define PAD_REFRESH_RATE 8         // ジョイパッドのI2C用データ更新頻度（単位ms）
#define JOYPAD_GENERALIZE 1        // ジョイパッドの入力値をPS系に一般化する（0:No, 1:Yes）
#define BT_REMOVE_BONDED_DEVICES 0 // 1でバインドデバイス情報クリア(BTリモコンがペアリング接続できない時に使用)

/* PINアサイン */
#define PIN_INFO_LED 25
#define PIN_RESET 33
// #define PIN_SPI_SCK 18
// #define PIN_SPI_MISO 36
// #define PIN_SPI_MOSI 26
// #define PIN_SPI_CS 19
