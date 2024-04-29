#define VERSION "Hello, This is Merimote PICO 2024.04.29"

/**                                                                     \
 * @file    Merimote_PICO/src/main.cpp                                  \
 * @brief   Merimote is a module for receiving remote control signals.  \
 *          It is part of the Meridian system.                          \
 *                                                                      \
 * This code is licensed under the MIT License.                         \
 * Copyright (c) 2024 Izumi Ninagawa                                    \
 */

//================================================================================================================
//---- 初 期 設 定  -----------------------------------------------------------------------------------------------
//================================================================================================================

#include "config.h"
#include "main.h"
#include <Arduino.h>
#include <Wire.h>
#include <Meridian.h>       // Meridianのライブラリ導入
MERIDIANFLOW::Meridian mrd; // ライブラリのクラスを mrdという名前でインスタンス化
#include <ESP32Wiimote.h>   // Wiiコントローラーのライブラリ
ESP32Wiimote wiimote;       // Wiiコントローラー設定
#include <PS4Controller.h>  // PS4コントローラー

/* PS4リモコンのためのペアリング情報設定用ライブラリ */
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
uint8_t pairedDeviceBtAddr[BT_PAIR_MAX_DEVICES][6];

/* システム用変数 */
TaskHandle_t thp[4]; // マルチスレッドのタスクハンドル格納用

/* フラグ関連変数 */
bool frag_pad_rcvd = true;   // UDPの受信終了フラグ
bool frag_pad_ready = false; // UDPの受信終了フラグ

/* タイマー管理用の変数 */
int joypad_polling_count = 0; // JOYPADのデータを読みに行くためのフレームカウント

/* リモコン用変数 */
UnionPad pad_array = {0};      // リモコン値格納用の配列
UnionPad pad_array_past = {0}; // リモコン値格納用の配列
UnionPad pad_i2c = {0};        //.bval = {11, 22, 33, 44, 55, 66, 77, 88, 127, 127}}; // ui64valを使用して全体を0で初期化
short pad_cksm_past = 0;       // 前回のボタンデータ（状態変化比較用）
unsigned short pad_btn = 0;
int pad_stick_R = 0;
int pad_stick_R_x = 0;
int pad_stick_R_y = 0;
int pad_stick_L = 0;
int pad_stick_L_x = 0;
int pad_stick_L_y = 0;
int pad_stick_V = 0;
int pad_R2_val = 0;
int pad_L2_val = 0;

/*I2Cでの返信*/
void onRequest()
{
  for (int i = 0; i < PAD_SHORT_SIZE * 2; i++)
  {
    Wire.write(pad_i2c.bval[i]);
  }
}

/*チェックサムデータの作成*/
short cksm_val(short arr[], int len)
{
  int _cksm = 0;
  for (int i = 0; i < len - 1; i++)
  {
    _cksm += int(arr[i]);
  }
  return short(~_cksm);
}

//================================================================================================================
//---- S E T  U P -----------------------------------------------------------------------------------------------
//================================================================================================================
void setup()
{
  /* ピンモードの設定 */
  pinMode(PIN_INFO_LED, OUTPUT); // 状態通知用LED
  pinMode(PIN_RESET, INPUT_PULLDOWN);    // リセットボタン用

  // Serial.begin(115200);
  Wire.onRequest(onRequest);
  Wire.begin((uint8_t)I2C_DEV_ADDR);
  Wire.setClock(100000); // 通信速度を400kHzに設定

  /* PC用シリアルの設定 */
  Serial.begin(SERIAL_PC_BPS);
  delay(1000);
  Serial.println(VERSION);
  delay(200);

  /* Bluetoothリモコン関連の処理 */
  bt_settings();

  /* JOYPADの認識 */
  mrd.print_controlpad(MOUNT_JOYPAD, PAD_REFRESH_RATE);

  /* Bluetoothの初期化 */
  uint8_t bt_mac[6];
  String self_mac_address = "";
  esp_read_mac(bt_mac, ESP_MAC_BT); // ESP32自身のBluetoothMacアドレスを表示
  self_mac_address = String(bt_mac[0], HEX) + ":" + String(bt_mac[1], HEX) + ":" + String(bt_mac[2], HEX) + ":" + String(bt_mac[3], HEX) + ":" + String(bt_mac[4], HEX) + ":" + String(bt_mac[5], HEX);
  Serial.print("ESP32's Bluetooth Mac Address is => " + self_mac_address);
  Serial.println();
  delay(200);

  /* スレッドの開始 */
  xTaskCreatePinnedToCore(Core0_BT_r, "Core0_BT_r", 4096, NULL, 5, &thp[2], 0);

  int i = 0;
  while (!frag_pad_ready)
  {
    if (i<50){
    digitalWrite(PIN_INFO_LED, true);
    }else{
      digitalWrite(PIN_INFO_LED, false);
    }
    i++;
    if(i>100){
      i = 0;
    }
    delay(10);
  }
  digitalWrite(PIN_INFO_LED, true);

  /* タイマーの調整と開始のシリアル表示 */
  Serial.println("-) Merimote system on M5StampPICO now flows. (-"); //
}

//================================================================================================================
//---- M A I N  L O O P -----------------------------------------------------------------------------------------
//================================================================================================================
void loop()
{
  // JOYPADの入力情報をI2C送信データに転記し, チェックサムを加える.
  for (int i = 0; i < PAD_SHORT_SIZE; i++)
  {
    pad_i2c.usval[i] = pad_array.usval[i];
  }
  pad_i2c.sval[PAD_SHORT_SIZE - 1] = cksm_val(pad_i2c.sval, PAD_SHORT_SIZE); // 末尾にチェックサムを挿入

  // 受信データのシリアルモニタ処理
  if (MONITOR_JOYPAD == 1)
  {
    if (pad_cksm_past != pad_i2c.sval[0])
    {
      Serial.println(pad_i2c.usval[0]);
    }
    pad_cksm_past = pad_i2c.sval[0];
  }
  else if (MONITOR_JOYPAD == 2)
  {
    Serial.print("btn:");
    Serial.print(pad_i2c.usval[0]);
    Serial.print(" Lstk:");
    Serial.print(pad_i2c.bval[2]);
    Serial.print(",");
    Serial.print(pad_i2c.bval[3]);
    Serial.print(" Rstk:");
    Serial.print(pad_i2c.bval[4]);
    Serial.print(",");
    Serial.print(pad_i2c.bval[5]);
    Serial.print(" L2a:");
    Serial.print(pad_i2c.ubval[6]);
    Serial.print(" R2a:");
    Serial.print(pad_i2c.ubval[7]);
    Serial.print(" cksm:");
    Serial.println(pad_i2c.sval[4], HEX);
  }
  delay(PAD_REFRESH_RATE);
  if (digitalRead(PIN_RESET)){
    ESP.restart();
  }
}

//================================================================================================================
//---- 関 数 各 種  -----------------------------------------------------------------------------------------------
//================================================================================================================
/**
 * @brief Pairing Bluetooth.
 *
 */
bool initBluetooth()
{

  if (!btStart())
  {
    Serial.println("Failed to initialize controller");
    return false;
  }

  if (esp_bluedroid_init() != ESP_OK)
  {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }

  if (esp_bluedroid_enable() != ESP_OK)
  {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
  return true;
}

/**
 * @brief Setting for PS4 Bluetooth.
 *
 */
void bt_settings()
{
  /* Bluetoothの初期化 */
  if (BT_REMOVE_BONDED_DEVICES) // ※常にBluetoothの初期化とアドレス表示を実行する設定
  {
    if (MOUNT_JOYPAD == 4)
    {
      initBluetooth();
      delay(100);
    }

    /* Bluetoothのペアリング情報 */
    int bt_count = esp_bt_gap_get_bond_device_num();
    if (!bt_count)
    {
      Serial.println("No bonded BT device found.");
    }
    else
    {
      Serial.print("Bonded BT device count: ");
      Serial.println(bt_count);
      if (BT_PAIR_MAX_DEVICES < bt_count)
      {
        bt_count = BT_PAIR_MAX_DEVICES;
        Serial.print("Reset bonded device count: ");
        Serial.println(bt_count);
      }
      esp_err_t tError = esp_bt_gap_get_bond_device_list(&bt_count, pairedDeviceBtAddr);

      if (ESP_OK == tError)
      {
        for (int i = 0; i < bt_count; i++)
        {
          Serial.print("Found bonded BT device # ");
          Serial.println(i);
          //Serial.print(" -> ");
          //Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
          if (BT_REMOVE_BONDED_DEVICES)
          {
            esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
            if (ESP_OK == tError)
            {
              Serial.print("Removed bonded BT device # ");
            }
            else
            {
              Serial.print("Failed to remove bonded BT device # ");
            }
            Serial.println(i);
          }
        }
      }
    }
  }

  /* PS4コントローラの接続開始 */
  if (MOUNT_JOYPAD == 4)
  {
    // PS4.begin(BT_MAC_ADDR); // ESP32のMACが入ります.PS4にも設定します.
    PS4.begin(); // ESP32のMACが入ります.PS4にも設定します.

    Serial.println("Try to connect PS4 controller...");
  }

  /* Wiiコントローラの接続開始 */
  if ((MOUNT_JOYPAD == 5) or (MOUNT_JOYPAD == 6))
  {
    Serial.println("Try to connect Wiimote...");
    wiimote.init();
  }
}

uint16_t pad_wiimote_receive()
{
  wiimote.task();
  if (wiimote.available() > 0)
  {
    static bool isFirstCall = true; // 初回の呼び出しフラグ
    if (isFirstCall)
    {
      Serial.println("Wiimote successfully connected. ");
      isFirstCall = false; // 初回の呼び出しフラグをオフにする
      frag_pad_ready = true;
    }
    uint16_t button = wiimote.getButtonState();
    pad_btn = 0;
    for (int i = 0; i < 16; i++)
    {
      uint16_t mask = 1 << i;
      if ((JOYPAD_GENERALIZE && (PAD_WIIMOTE_SOLO[i] & button)) || (!JOYPAD_GENERALIZE && (PAD_WIIMOTE_ORIG[i] & button)))
      {
        pad_btn |= mask;
      }
    }
    pad_array.usval[0] = pad_btn; // short型で4個の配列データを持つ
    return pad_btn;
  }
  else
  {
    return pad_btn;
  }

  if (MOUNT_JOYPAD == 6)
  {
    Serial.println("Wiimote + Nunchuk not available now. "); //
  }
}

/**
 * @brief Receive input values from the PS4 remote control
 *        and store them in the following variables:
 *        pad_btn, pad_L2_val, pad_R2_val, pad_stick_L, pad_stick_R, pad_stick_V
 */
void pad_ps4_receive()
{
  // Below has all accessible outputs from the controller
  if (PS4.isConnected())
  {
    static bool isFirstCall = true; // 初回の呼び出しフラグ
    if (isFirstCall)
    {
      Serial.println("PS4 controller successfully connected. ");
      isFirstCall = false; // 初回の呼び出しフラグをオフにする
      frag_pad_ready = true;
    }
    pad_btn = 0;

    if (JOYPAD_GENERALIZE)
    { // 一般化,ROS準拠
      if (PS4.Up())
        pad_btn |= (0b00000000 * 256) + 0b00010000; // 16
      if (PS4.Right())
        pad_btn |= (0b00000000 * 256) + 0b00100000; // 32
      if (PS4.Down())
        pad_btn |= (0b00000000 * 256) + 0b01000000; // 64
      if (PS4.Left())
        pad_btn |= (0b00000000 * 256) + 0b10000000; // 128
      if (PS4.Triangle())
        pad_btn |= (0b00010000 * 256) + 0b00000000; // 4096
      if (PS4.Circle())
        pad_btn |= (0b00100000 * 256) + 0b00000000; // 8192
      if (PS4.Cross())
        pad_btn |= (0b01000000 * 256) + 0b00000000; // 16384
      if (PS4.Square())
        pad_btn |= (0b10000000 * 256) + 0b00000000; // 32768
      if (PS4.UpRight())
        pad_btn |= (0b00000000 * 256) + 0b00110000; // 48
      if (PS4.DownRight())
        pad_btn |= (0b00000000 * 256) + 0b01100000; // 96
      if (PS4.UpLeft())
        pad_btn |= (0b00000000 * 256) + 0b10010000; // 144
      if (PS4.DownLeft())
        pad_btn |= (0b00000000 * 256) + 0b11000000; // 192
      if (PS4.Share())
        pad_btn |= (0b00000000 * 256) + 0b00000001; // 1
      if (PS4.L3())
        pad_btn |= (0b00000000 * 256) + 0b00000010; // 2
      if (PS4.R3())
        pad_btn |= (0b00000000 * 256) + 0b00000100; // 4
      if (PS4.Options())
        pad_btn |= (0b00000000 * 256) + 0b00001000; // none
      if (PS4.L1())
        pad_btn |= (0b00000100 * 256) + 0b00000000; // 1024
      if (PS4.R1())
        pad_btn |= (0b00001000 * 256) + 0b00000000; // 2048

      if (PS4.L2())
      {
        pad_btn |= (0b00000001 * 256) + 0b00000000;
        pad_L2_val = constrain(PS4.L2Value(), 0, 255); // 256
      }
      if (PS4.R2())
      {
        pad_btn |= 512;                                //(0x00000010 * 256) + 0b00000000;
        pad_R2_val = constrain(PS4.R2Value(), 0, 255); // 512
      }

      if (PS4.PSButton())
        pad_btn |= (0b00000000 * 256) + 0b01010000; // same as up & down
      if (PS4.Touchpad())
        pad_btn |= (0b00000000 * 256) + 0b10100000; // same as left & right

      if (PS4.LStickX())
      {
        pad_stick_L_x = constrain(PS4.LStickX(), -127, 127);
      }
      if (PS4.LStickY())
      {
        pad_stick_L_y = constrain(PS4.LStickY(), -127, 127);
      }
      if (PS4.RStickX())
      {
        pad_stick_R_x = constrain(PS4.RStickX(), -127, 127);
      }
      if (PS4.RStickY())
      {
        pad_stick_R_y = constrain(PS4.RStickY(), -127, 127);
      }
    }
    else
    { // ストレート
      if (PS4.Up())
        pad_btn |= (0b00000000 * 256) + 0b00010000; // 16
      if (PS4.Right())
        pad_btn |= (0b00000000 * 256) + 0b00100000; // 32
      if (PS4.Down())
        pad_btn |= (0b00000000 * 256) + 0b01000000; // 64
      if (PS4.Left())
        pad_btn |= (0b00000000 * 256) + 0b10000000; // 128
      if (PS4.Triangle())
        pad_btn |= (0b00010000 * 256) + 0b00000000; // 4096
      if (PS4.Circle())
        pad_btn |= (0b00100000 * 256) + 0b00000000; // 8192
      if (PS4.Cross())
        pad_btn |= (0b01000000 * 256) + 0b00000000; // 16384
      if (PS4.Square())
        pad_btn |= (0b10000000 * 256) + 0b00000000; // 32768
      if (PS4.UpRight())
        pad_btn |= (0b00000000 * 256) + 0b00110000; // 48
      if (PS4.DownRight())
        pad_btn |= (0b00000000 * 256) + 0b01100000; // 96
      if (PS4.UpLeft())
        pad_btn |= (0b00000000 * 256) + 0b10010000; // 144
      if (PS4.DownLeft())
        pad_btn |= (0b00000000 * 256) + 0b11000000; // 192
      if (PS4.L1())
        pad_btn |= (0b00000100 * 256) + 0b00000000; // 1024
      if (PS4.R1())
        pad_btn |= (0b00001000 * 256) + 0b00000000; // 2048
      if (PS4.Share())
        pad_btn |= (0b00000000 * 256) + 0b00000001; // 1
      if (PS4.R3())
        pad_btn |= (0b00000000 * 256) + 0b00000010; // 2
      if (PS4.L3())
        pad_btn |= (0b00000000 * 256) + 0b00000100; // 4
      if (PS4.Options())
        pad_btn |= (0b00000000 * 256) + 0b00001000; // 8
      // if (PS4.PSButton())
      //   pad_btn |= (0b00000000 * 256) + 0b01010000; // same as up & down
      // if (PS4.Touchpad())
      //   pad_btn |= (0b00000000 * 256) + 0b10100000; // same as left & right
      if (PS4.L2())
      {
        pad_btn |= (0b00000001 * 256) + 0b00000000;
        pad_L2_val = constrain(PS4.L2Value(), 0, 255);
      }
      if (PS4.R2())
      {
        pad_btn |= 512; //(0x00000010 * 256) + 0b00000000;
        pad_R2_val = constrain(PS4.R2Value(), 0, 255);
      }
      if (PS4.LStickX())
      {
        pad_stick_L_x = constrain(PS4.LStickX(), -127, 127);
      }
      if (PS4.LStickY())
      {
        pad_stick_L_y = constrain(PS4.LStickY(), -127, 127);
      }
      if (PS4.RStickX())
      {
        pad_stick_R_x = constrain(PS4.RStickX(), -127, 127);
      }
      if (PS4.RStickY())
      {
        pad_stick_R_y = constrain(PS4.RStickY(), -127, 127);
      };
    }

    pad_array.usval[0] = pad_btn;                            // short型で4個の配列データを持つ
    pad_array.sval[1] = pad_stick_L_x * 256 + pad_stick_L_y; // short型で4個の配列データを持つ
    pad_array.sval[2] = pad_stick_R_x * 256 + pad_stick_R_y; // short型で4個の配列データを持つ
    pad_array.sval[3] = pad_R2_val * 256 + pad_L2_val;       // short型で4個の配列データを持つ
  }
}

void monitor_joypad(ushort *arr)
{
  for (int i = 0; i < 4; i++)
  {
    Serial.print(arr[i]);
    if (i < 3)
    {
      Serial.print("/");
    }
  }
  Serial.println();
}

//================================================================================================================
//---- Bluetooth 用 ス レ ッ ド -----------------------------------------------------------------------------------
//================================================================================================================
void Core0_BT_r(void *args)
{ // サブCPU(Core0)で実行するプログラム
  int _wait = PAD_REFRESH_RATE - 1;
  if (_wait < 0)
  {
    _wait = 0;
  }

  while (true)
  { // Bluetooth待受用の無限ループ
    // PS4 controller
    if (MOUNT_JOYPAD == 4)
    {
      pad_ps4_receive();
    }
    // Wiimote
    if ((MOUNT_JOYPAD == 5) or (MOUNT_JOYPAD == 6))
    {
      pad_wiimote_receive();
    }
    // frag_pad_rcvd = true;
    delay(_wait); // _wait ms
  }
  delay(1); // 1ms
}
