# セグメントLED制御用ミドルウェア
## 1. 概要
セグメントLED制御用ミドルウェアモジュールについて説明する。  
本モジュールは、数字情報等の表示に特化したセグメントLEDの制御用ソフトウェアとして定義する。  

### 1.1 目的
セグメントLEDがもつ機能を抽象化し、ユーザーアプリケーションがマイコン・HW環境に依存しないソフトウェアモジュールの提供を目的とする。  
本モジュールが提供する機能を以下に示す。  

* セグメントLEDの主制御  
* HW依存による制御方式の差分吸収  
(スタティック・ダイナミック制御に対応)
* 表示周期管理
* 表示状態管理  
常時表示・非表示・点滅表示の各状態を桁単位で設定することが可能  


### 1.2 注意事項
本モジュールはHW制御を行わない。  
描画処理はユーザー定義のコールバック関数に委譲される。  

## 2. ソフトウェア構成
### 2.1 モジュール階層
ユーザーアプリケーションとHWドライバーの中間に位置するミドルウェアモジュールとしての使用を想定している。  
以下にモジュール階層図を記載する。  

```text
APP
 ↑
segment middleware
 ↑
HW driver
 ↑
mcu
```

### 2.2. ファイル構成
本モジュールのファイル構成を以下に示す。

```text
segment/
  ├── segment.c   <- 制御機能を定義
  └── segment.h   <- 公開APIを定義
```

### 2.3. API

API関数一覧を以下に示す。

| API | 説明 | 備考 |
|:--|---|:-:|
| seg_init() | 初期化 | - |
| seg_set_draw_cb() | 描画コールバック設定 | (Note1) |
| seg_set_encode_cb() | エンコードコールバック設定 | (Note2) |
| seg_set_static_control() | スタティック制御設定 | - |
| seg_set_dynamic_control() | ダイナミック制御設定 | - |
| seg_update() | 状態・周期更新 | (Note3) |
| seg_set_text() | 表示データ設定(text) | - |
| seg_set_pattern() | 表示データ設定(bitパターン) | - |
| seg_hidden_on() | 非表示ON | - |
| seg_hidden_off() | 非表示OFF | - |
| seg_blink_on() | 点滅ON | (Note3) |
| seg_blink_off() | 点滅OFF | - |
| seg_set_scan_cycle() | 描画更新周期設定(ダイナミック制御用) | - |

* Note  
1. 本コールバックの登録がされていない場合は、セグメントLEDの表示・更新がされない。  
表示制御を行うHWドライバーの登録を行うこと。
2. 表示データ設定(text)を使用する場合、本コールバックの登録が必要となる。  
textに応じたbitパターンをユーザーが生成する。
3. 本モジュールは内部で周期管理を行っている。  
seg_updateの引数「period」に任意の値を指定することで周期の更新が可能。  
periodには前回呼び出しからの経過時間(ms)を指定し、経過時間は1ms以上を推奨する。  

### 2.4. 使用例
4桁のセグメントLEDをダイナミック制御で動作させる実装例を以下に示す。  
また、「example」配下にサンプルコード・動作例を格納する  


```c
static seg_ctx_t segment;

void display_init(void)
{
    /* セグメントLED初期化 */
    seg_init(&segment, 4);                  /* 4桁のLEDを使用 */
    seg_set_dynamic_control(&segment);      /* ダイナミック制御 */

    /* 描画コールバックとエンコードコールバックの設定 */
    seg_set_draw_cb(&segment, draw_cb);
    seg_set_encode_cb(&segment, encode_cb);

    /* LEDに"2026"と表示 */
    seg_set_text(&segment, "2026");

    /* 点滅ON */
    seg_blink_on(&segment, 0, 500);         /* blink 500ms cycle */
}

/***
 * @brief 表示更新割り込み
 * @details タイマ割り込みなどで呼び出す。
 *          表示更新周期は1msを推奨。
 */
void display_interrupt(void)
{
    /* 表示更新 */
    seg_update(&segment, 1);    /* 1ms period inc */
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

## 3. 変更履歴
| Version | Date | 説明 |
|---|---|---|
| v1.0.0 | 2026-05-29 | 新規作成 |