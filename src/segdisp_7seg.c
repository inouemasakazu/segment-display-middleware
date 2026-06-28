/****************************************************************************************************
 * @file    segdispp_7seg.c
 * @brief   7セグメントディスプレイ制御用モジュール
 * @details このファイルではセグメントディスプレイ用ミドルウェアモジュールに対応する7セグ用機能を定義。
 *
 * @author  Masakazu Inoue
 * @date    2026/06/28          新規作成
 ****************************************************************************************************/

/****************************************************************************************************
 * Private include
 ****************************************************************************************************/
#include "segdisp_7seg.h"

#include <string.h>
#include <stdbool.h>

/****************************************************************************************************
 * Private define
 ****************************************************************************************************/
/*** 7セグで表現可能なbitパターン ***/
#define SEGDISP_7SEG_0                  0x3F
#define SEGDISP_7SEG_1                  0x06
#define SEGDISP_7SEG_2                  0x5B
#define SEGDISP_7SEG_3                  0x4F
#define SEGDISP_7SEG_4                  0x66
#define SEGDISP_7SEG_5                  0x6D
#define SEGDISP_7SEG_6                  0x7D
#define SEGDISP_7SEG_7                  0x07
#define SEGDISP_7SEG_8                  0x7F
#define SEGDISP_7SEG_9                  0x6F

#define SEGDISP_7SEG_DP                 0x80

/****************************************************************************************************
 * Private typedef
 ****************************************************************************************************/

/****************************************************************************************************
 * Private Global Variables
 ****************************************************************************************************/

/**
 * @brief 7セグで表現可能な数値(0～9)
 */
static uint32_t segdisp_7seg_number[] = {
    SEGDISP_7SEG_0,
    SEGDISP_7SEG_1,
    SEGDISP_7SEG_2,
    SEGDISP_7SEG_3,
    SEGDISP_7SEG_4,
    SEGDISP_7SEG_5,
    SEGDISP_7SEG_6,
    SEGDISP_7SEG_7,
    SEGDISP_7SEG_8,
    SEGDISP_7SEG_9
};


/****************************************************************************************************
 * Private Prototype Declaration
 ****************************************************************************************************/
static uint8_t segdisp_7seg_encode_cb(const uint8_t *c, uint32_t *pattern);
static uint8_t segdisp_7seg_encode_char(const uint8_t *c, uint32_t *pattern);

/**
 * @brief 7セグ用初期化
 * @param segdisp セグメントディスプレイの制御データを保持するメモリ領域
 */
void segdisp_7seg_init(segdisp_t *segdisp)
{
    /* 7セグ用エンコードCB設定 */
    segdisp->encode_cb = segdisp_7seg_encode_cb;
}

/**
 * @brief 7セグ用エンコードCB
 * @param c       エンコード前の文字列を格納するポインタ
 * @param pattern エンコード後のbitパターンを格納するポインタ
 * @return データ長
 */
static uint8_t segdisp_7seg_encode_cb(const uint8_t *c, uint32_t *pattern)
{
    uint8_t length = 0;

    if ((c == NULL) || (pattern == NULL))
    {
        length = 0;
    }
    else
    {
        /* エンコード処理の実行 */
        length = segdisp_7seg_encode_char(c, pattern);
    }

    return length;
}

/**
 * @brief 7セグ用エンコード処理
 *        7セグで表現可能なbitパターンへのエンコード(文字列→bitパターン)を行う。
 * @param c       エンコード前の文字列を格納するポインタ
 * @param pattern エンコード後のbitパターンを格納するポインタ
 * @return データ長
 */
static uint8_t segdisp_7seg_encode_char(const uint8_t *c, uint32_t *pattern)
{
    uint32_t bit = 0;
    uint8_t index = 0;
    uint8_t size = 0;

    for (size_t i = 0; i < SEGDISP_DIGIT_MAX; i++)
    {
        if ('\0' == c[i])
        {
            break;
        }

        /* パターン変換 */
        if (('0' <= c[i]) && (c[i] <= '9'))
        {
            index = c[i] - '0';
            bit = segdisp_7seg_number[index];
        }
        else if (('A' <= c[i]) && (c[i] <= 'Z'))
        {
            bit = 0x40;
        }
        else if (('a' <= c[i]) && (c[i] <= 'z'))
        {
            bit = 0x40;
        }
        else if ('.' == c[i])
        {
            if (0 < size)
            {
                if (SEGDISP_7SEG_DP == pattern[size - 1])
                {
                    bit = SEGDISP_7SEG_DP;
                }
                else
                {
                    size--;
                    bit = pattern[size] | SEGDISP_7SEG_DP;
                }
            }
            else
            {
                bit = SEGDISP_7SEG_DP;
            }
        }
        else
        {
            /* 例外のためbitパターンなし */
            bit = 0x00;
        }

        /* 8bitで表現可能(segment + DP)な範囲にマスクする */
        pattern[size] = bit & 0xff;
        size++;
    }

    return size;
}
