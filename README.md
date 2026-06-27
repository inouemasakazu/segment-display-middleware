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
  |     ├── segdisp_7seg.c
  |     ├── segdisp_7seg.h
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
1桁の典型的な7セグメントディスプレイを制御するサンプルコードを以下に記載します。  

```c
static segdisp_t segment;

/***
 * @brief 7セグ初期化・描画開始
 */
void sample(void)
{
    /* セグメントLED初期化 */
    segdisp_init(&segment, 1);              /* 1桁のLEDを使用 */
    segdisp_set_static_control(&segment);   /* スタティック制御 */

    /* 描画CBの設定 */
    segdisp_set_draw_cb(&segment, draw_cb);

    /* 7セグメントディスプレイに"0."を表示する */
    segdisp_set_text(&segment, "0.");

    /* 表示更新 */
    segdisp_update(&segment, 1);    /* 1ms period inc */

    while(1);
}

/***
 * @brief 描画CB
 */
static void draw_cb(const uint32_t *data, uint8_t len, uint8_t pos)
{
    uint8_t byte = (uint8_t)*data;
    /* HWの仕様に合わせてユーザーが定義する。 */
}
```