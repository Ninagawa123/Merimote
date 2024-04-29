#ifndef __MERIDIAN_LOCAL_FUNC__
#define __MERIDIAN_LOCAL_FUNC__

#include <Arduino.h>
#include <config.h>
#include "esp_gap_bt_api.h"

//================================================================================================================
//---- クラス・構造体・共用体・定義数  ---------------------------------------------------------------------------------
//================================================================================================================
/* システム用定義数 */
#define PAD_SHORT_SIZE 5       // ジョイパッドのデータ格納数
#define BT_PAIR_MAX_DEVICES 20 // BT接続デバイスの記憶可能数

/* リモコン用変数 */
typedef union
{
    short sval[PAD_SHORT_SIZE + 2]; // short型で4個の配列データを持つ
    uint16_t usval[PAD_SHORT_SIZE + 2]; // 上記のunsigned short型
    int8_t bval[(PAD_SHORT_SIZE +2)*2];    // 上記のbyte型
    uint8_t ubval[(PAD_SHORT_SIZE +2)*2];    // 上記のunsigned byte型
    uint64_t ui64val[1]; // 上記のunsigned int16型
                         // button, pad.stick_L_x:pad.stick_L_y,
                         // pad.stick_R_x:pad.stick_R_y, pad.L2_val:pad.R2_val
} UnionPad;

struct PadValue // リモコンの値
{
    unsigned short btn = 0;
    int stick_R = 0;
    int stick_R_x = 0;
    int stick_R_y = 0;
    int stick_L = 0;
    int stick_L_x = 0;
    int stick_L_y = 0;
    int stick_V = 0;
    int R2_val = 0;
    int L2_val = 0;
};

/* リモコン変換テーブル */
constexpr unsigned short PAD_WIIMOTE_SOLO[16] = {0x1000, 0x0080, 0x0000, 0x0010, 0x0200, 0x0400, 0x0100, 0x0800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0001, 0x0002, 0x0004};
constexpr unsigned short PAD_WIIMOTE_ORIG[16] = {0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0000, 0x0000, 0x0080};

//================================================================================================================
//---- 関 数 各 種  -----------------------------------------------------------------------------------------------
//================================================================================================================

/**
 * @brief Receive input values from the wiimote
 *        and store them in pad_btn.
 */
uint16_t pad_wiimote_receive();

/**
 * @brief Setting for PS4 Bluetooth.
 *
 */
void bt_settings();

/**
 * @brief Thread for Bluetooth communication processing.
 *
 * @param args ?
 */
void Core0_BT_r(void *args);

/**/
//void bda2str(const esp_bd_addr_t bda, char *str, size_t size);

#endif
