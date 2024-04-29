//Merimote Receiver for Arduino UNO

#include <Wire.h>
#define I2C_DEV_ADDR 0x58
#define PAD_SHORT_SIZE 5

// 表示設定
// 0:ボタンに変化があった時のみ（ボタンテスト用）
// 1:常に表示（アナログレバーのテスト用）
#define MONITOR 0  

uint32_t i = 0;

typedef union
{
  short sval[PAD_SHORT_SIZE + 2]; // short型で4個の配列データを持つ
  uint16_t usval[PAD_SHORT_SIZE + 2]; // 上記のunsigned short型
  int8_t bval[(PAD_SHORT_SIZE + 2) * 2]; // 上記のbyte型
  uint8_t ubval[(PAD_SHORT_SIZE + 2) * 2]; // 上記のunsigned byte型
  uint64_t ui64val[1]; // 上記のunsigned int16型
  // button, pad.stick_L_x:pad.stick_L_y,
  // pad.stick_R_x:pad.stick_R_y, pad.L2_val:pad.R2_val
} UnionPad;

UnionPad pad_data;
short pad_data_past = 0;

short cksm_val(short arr[], int len)
{
  int _cksm = 0;
  for (int i = 0; i < len - 1; i++)
  {
    _cksm += int(arr[i]);
  }
  return short(~_cksm);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000); // 通信速度を100kHzに設定
  delay(1000);
  Serial.println("---- Merimote I2C test ----");
}

void loop() {
  delay(20);

  Wire.requestFrom(I2C_DEV_ADDR, 10);
  i = 0;
  while (Wire.available()) { // バッファにデータがある間
    //byte c = Wire.read();  // バッファから1バイト読み取り
    pad_data.bval[i] = Wire.read();  // バッファから1バイト読み取り
    i ++;
  }

  // 表示
  if ((pad_data_past != pad_data.sval[0]) or MONITOR) {
    ///Serial.print(pad_data.sval[0], BIN);
    Serial.print("Btn:");
    for (int i = 15; i >= 0; i--) {
      Serial.print((pad_data.sval[0] & (1 << i)) ? 'o' : '-');
    }
    Serial.print(" Lx:");
    Serial.print(pad_data.bval[3], DEC);//リトルエンディアンに変換
    Serial.print(" Ly:");
    Serial.print(pad_data.bval[2], DEC);
    Serial.print(" Rx:");
    Serial.print(pad_data.bval[5], DEC);
    Serial.print(" Ry:");
    Serial.print(pad_data.bval[4], DEC);
    Serial.print(" Lv:");
    Serial.print(pad_data.ubval[6], DEC);
    Serial.print(" Rv:");
    Serial.print(pad_data.ubval[7], DEC);
    Serial.print(" Cksm:");
    if (cksm_val(pad_data.sval, PAD_SHORT_SIZE) == pad_data.sval[4])
    {
      Serial.print("Ok");
    } else {
      Serial.print("*NG");
    }
    Serial.println();
  }
  pad_data_past = pad_data.sval[0];
}
