# segment-display-middleware

## 目次
1. [概要](#概要)
2. [ディレクトリ構成](#ディレクトリ構成)
3. [API](#API)
4. [使用例](#使用例)
5. [備考](#備考)


## 概要
「segment-display-middleware」では数字情報等の表示に特化した、
セグメントLEDを制御するためのミドルウェアモジュールを提供します。  


### 目的
セグメントLEDがもつ機能を抽象化し、ユーザーアプリケーションがマイコン・HW環境に依存しないソフトウェアモジュールの提供を目的とする。  
本モジュールが提供する機能を以下に示す。  

* セグメントLEDの主制御  
* HW依存による制御方式の差分吸収  
(スタティック・ダイナミック制御に対応)
* 表示周期管理
* 表示状態管理  
常時表示・非表示・点滅表示の各状態を桁単位で設定することが可能  


### 注意事項
本モジュールはHW制御を行わない。  
描画処理はユーザー定義のコールバック関数に委譲される。  


## ディレクトリ構成
本モジュールのディレクトリ構成を以下に示す。
```text
segment-display-middleware/
  ├── example/
  |     ├── demo_segment_led/
  |     |    ├── Core/
  |     |    ├── Drivers/
  |     |    └── segment_led/
  |     |         ├── CLI/
  |     |         ├── u74hc595/
  |     |         ├── UART/
  |     |         ├── cmd.c
  |     |         ├── cmd.c
  |     |         ├── display.c
  |     |         └── display.h
  |     ├── docs/
  |     └── RAEDME.md
  ├── inc/
  |     └── segdisp.h
  ├── src/
  |     └── segdisp.c
  └── RAEDME.md
```

### モジュール階層
ユーザーアプリケーションとHWドライバーの中間に位置するミドルウェアモジュールとしての使用を想定している。  
以下にモジュール階層図を記載する。  

```text
APP
 ↑
segment-display-middleware
 ↑
HW driver
 ↑
mcu
```


## 使用例
1桁の7セグメントディスプレイを制御するサンプルコードを以下に記載する。  

```c
static segdisp_t segment;

void sample(void)
{
    /* セグメントLED初期化 */
    segdisp_init(&segment, 1);              /* 1桁のLEDを使用 */
    segdisp_set_static_control(&segment);   /* スタティック制御 */

    /* 描画コールバックとエンコードコールバックの設定 */
    segdisp_set_draw_cb(&segment, draw_cb);
    segdispset_encode_cb(&segment, encode_cb);

    /* 7セグメントディスプレイに"0."を表示する */
    segdisp_set_text(&segment, "0.");

    /* 表示更新 */
    segdisp_update(&segment, 1);    /* 1ms period inc */

    while(1);
}

/***
 * @brief 描画コールバック
 */
static void draw_cb(const uint32_t *data, uint8_t len, uint8_t pos)
{
    uint8_t byte = (uint8_t)*data;
    /* HWの仕様に合わせてユーザーが定義する。 */
}

/***
 * @brief エンコードコールバック
 * @details 7セグメントLEDの表示部(点灯部)に合わせて、
 *          textデータから任意のbitパターンにエンコードを行う。
 *
 *          実装例ではtextから、数字・小数点(ドット)に対応したbitパターンにエンコードしている。
 */
static uint8_t encode_cb(const uint8_t *c, uint32_t *pattern)
{
    /* 7segment led用bitパターン */
    uint32_t num_bit[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };

    uint8_t length = 0;

    for (size_t i = 0; i < SEG_DIGIT_LENGTH; i++)
    {
        if (c[i] == '\0')
        {
            /* 文字列終端 */
            break;
        }
        else
        {
            /* パターン変換 */
            if (('0' <= c[i]) && (c[i] <= '9'))
            {
                /* 数字はbitパターンに変換して格納 */
                pattern[length] = num_bit[(c[i] - '0')];
                length++;
            }
            else if (c[i] == '.')
            {
                /* 小数点はbitパターンの8bit目を点灯 */
                if (length == 0)
                {
                    pattern[length] |= 0x80;
                    length++;
                }
                else
                {
                    pattern[(length - 1)] |= 0x80;
                }
            }
            else
            {
                /* 数字じゃない場合は例外扱い */
                pattern[length] = 0x00;
                length++;
            }
        }
    }

    return length;
}
```