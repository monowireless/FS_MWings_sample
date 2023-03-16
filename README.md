# MWings_Sample

ESP8266 または EPS32 で、無線マイコン TWELITE からの無線パケット出力を処理するサンプルです。
また Azure IoT Hub 接続のサンプルも兼ねています(定義によりAzureなしでもビルドできます)。

## 接続方法
TWELITE と ESP は UART　(シリアル) の接続を行っておきます。
* ESP8266の場合は UART0 (Serial IO1,3) を用います。
* ESP32の場合は UART2 (Serial2 IO16,17) を用います。

```
TWELITE UART RX --- ESP TX
        UART TX ---     RX
```

詳細は MWings ライブラリの解説を参照してください。

## ライブラリ
以下のライブラリが必要です。

* [Azure-IoTHub-MQTT-ESP](https://github.com/monowireless/FS_Azure-IoTHub-MQTT-ESP) - Azure 接続用のライブラリ
* [MWings](https://github.com/monowireless/mwings) - TWELITE AppWings シリアルパーサ（シリアルからのデータ列を処理する）ライブラリ
* [MWutls](https://github.com/monowireless/mwutls) - EPS用の汎用ライブラリ (printfメッセージの出力など)

本サンプル内には以下が含まれます。
* [OTFFT](http://wwwa.pikara.ne.jp/okojisan/stockham/optimization1.html) - OK おじさん氏作成のFFTサンプルコード（インタフェースの整備、単精度float用に計算速度の調整を行っています)



## Wifi, Azure 接続キー

`x_my_connect_template.h`を`x_my_connect.h`にコピーして使用します。

自身の環境に合わせて書き換えてください。
```C
// note: SSID and KEY
#define MY_WIFI_SSID "MY_SSID"       // ← WIFIのSSID
#define MY_WIFI_KEY "MY_SSID_PASSWD" // ← WIFIのパスワード

// note: AZURE connect key
#define MY_CONNECT_KEY "__PASTE_AZURE_CONNECT_KEY__" // ← Azureの接続キー
```



## ファイル一覧

| ファイル名                | 解説                                                         | 備考 |
| ------------------------- | ------------------------------------------------------------ | ---- |
| `MWings_Sample.ino`       | `setup()`, `loop()` 主要処理。                               |      |
| `x_my_connect.h`          | WIFI, Azureの設定。`x_my_connect_template.h`からコピーして、環境に合わせて書き換えてください。 |      |
| `x_my_connect_template.h` |                                                              |      |
| `azure.cpp`               | Azure 関連のコールバックなど諸関数。あまり重要な処理は含まれません。 |      |
| `common.h`                | ハードウェア定義（ポート番号）、関数プロトタイプ宣言など。   |      |
| `otfft.hpp`, `otfft.cpp`  | DFT(FFT)処理。組み込み用に専用化されたものではないが、64,128サンプル程度であれば実用的な速度(64サンプルをESP8266で約5ms、3軸で15ms)で処理できる。`sensor_pal_mot.cpp` で利用する。 |      |
| `pkt_sequence.hpp`        | 連続した無線パケットを管理するための手続き。TWELITE 無線パケットには１パケット当たり10～16サンプル格納されるが、FFTを行うには32,64,128パケットといったより長いサンプルが必要。受信パケットの続き番号をもとに連続パケットか否かを判定する。 |      |
| `sensor_aria.cpp`         | TWELITE ARIA(温湿度センサー)のパケットを処理する。ESPのSerialに受信パケット(温度湿度など)の情報を表示する。<br />`#undef DEBUG_UNUSE_AZURE`の場合：Azureにデータ(Sensro: ARIA, SID: モジュールシリアルID, ValueA: 温度[℃], ValueB: 湿度[%])をプッシュする。 |      |
| `sensor_cue.cpp`          | TWELITE CUE(加速度センサー)のパケットを処理する。ESPのSerialに受信パケットの情報(パケット中の10サンプルのX,Y,Z加速度の平均値など)を表示する。<br />`#undef DEBUG_UNUSE_AZURE`の場合：Azureにデータ(Sensro: CUE, SID: モジュールシリアルID, ValueA: X軸平均値[G], ValueB: Y軸平均値[G], ValueC: Z軸平均値[G])をプッシュする。 |      |
| `sensor_pal_amb.cpp`      | TWELITE AMB PAL (温湿度、照度)のパケットを処理する。ESPのSerialに受信パケット(温度湿度照度など)の情報を表示する。<br />`#undef DEBUG_UNUSE_AZURE`の場合：Azureにデータ(Sensro: PAL_AMB, SID: モジュールシリアルID, ValueA: 温度[℃], ValueB: 湿度[%], ValueC: 照度[lx])をプッシュする。 |      |
| `sensor_pal_mot.cpp`      | TWELITE CUE(MOTモード設定), TWELITE MOT PALのパケットを処理する。連続した64サンプルが受信できた時点で`do_FFT()`を実行し、結果は`vfft_abs[0..2]`に格納される。さらに、各軸のピーク値上位３つ`pks_valuse[0..2:軸][0..2:ピーク]`とその周波数インデックス`pks_idx[0..2:軸]`を探索する。<br />`#undef DEBUG_UNUSE_AZURE`の場合：Azureにデータ(Sensro: PAL_MOT, SID: モジュールシリアルID, ValueA: X軸ピーク[G], ValueB: Y軸ピーク[G], ValueC: Z軸ピーク, AttrA: X軸周波数インデックス, AttrB: Y軸周波数インデックス, AttrC: Z軸周波数インデックス)をプッシュする。 |      |
| `sensor_util.cpp`         | 整数を100で割った値を表示する`print_div100()`関数(例 2345->"23.45")。 |      |



## 解説

MWings ライブラリは[こちらの解説](doc/mwings_quick.md)も参照してください。

### common.h
各種定義を行います。

```C++
#define DEBUG_UNUSE_AZURE // 定義すると Azure 接続関連をビルドしません。コメントアウトすると Azure 接続します。

//// SELECT BOARD (Choose ONE)     // ボードの選択です。
#define BRD_DEFAULT                // ← 他のビルド定義から BRD_WIONODE または BRD_TWELITE_SPOT が選択されます。
//#define BRD_WIONODE              // ← ESP8266 WIONODE 用
//#define BRD_TWELITE_SPOT          // ← ESP32 用

//// OTFFT DATYTYPE
using REAL = float; // ← FFTなどで使用するときの浮動小数点計算の実数型を指定します。
                    //   精度が必要な場合は double を指定しますが計算時間が２倍またはそれ以上(ESP32ではハード
                    //   ウェア支援がなくなるのでさらに大きな差になる)になります。
```

また、本ファイルでは C, C++ のプロトタイプ宣言を明示的に行っています。
```C++
//// Prototypes
void on_pkt_ARIA(tsMWings_App_ARIA &d);
void on_pkt_CUE(tsMWings_App_CUE &d);
void on_pkt_PAL_AMB(tsMWings_App_PAL_AMB &d);
void on_pkt_PAL_MOT(tsMWings_App_PAL_MOT &d);

void print_div100(uint16_t val);
void print_div100(int16_t val);

void azureCallback(String s);
```

末尾には各ボード用の所定義が含まれます。
```C++
#if defined(BRD_WIONODE)
#define ARCH_ESP8266     // ESP8266 is also defined at board support
const uint8_t PORT0A = 1;
const uint8_t PORT0B = 3;
...
#endif

//// FOR ESP32
#if defined(BRD_TWELITE_SPOT)
#define ARCH_ESP32       // ESP32 is also defiend at board support
const uint8_t PORT_TWE_RST = 5;
const uint8_t PORT_TWE_PGM = 4;
const uint8_t BLUE_LED = 18;
#endif
```

### MWings_Sample.ino
`setup()`, `loop()`記述のメインファイル。



#### 宣言部

ライブラリ利用宣言と `twelite` オブジェクトの宣言が含まれます。

```C++
// for TWELITE
#include <MWings.h> // TWELITE Wings シリアルパーサー (new!)
#include <MWutls.h> // ユーティリティ (WSer, format()他)

// -------------
// クラスオブジェクト
MWings twelite;
```



#### setup()
1. `Serial` の初期化やポートの初期化

   ```C++
   Serial.begin(115200);
   
   #if defined(BRD_WIONODE)
   	// GROVE の VCC に電源供給するための指定
   	pinMode(PORT_POWER, OUTPUT);
   	digitalWrite(PORT_POWER, HIGH);
   #endif
   ```
2. WiFi の接続
3. (ESP32) `Serial2`の初期化
4. MWings の初期化 (シリアルパーサ)
   ```C++
   // twelite の初期化
   #if defined(BRD_WIONODE)
     twelite.setup(Serial); // MWings の初期化
   #elif defined(BRD_TWELITE_SPOT)
     twelite.debugUsing(Serial); // デバッグメッセージの出力設定
     twelite.setup(Serial2, PORT_TWE_RST, PORT_TWE_PGM); // MWings の初期化
   #endif
   
   // 無線アプリケーションごとに処理する関数を登録します。
   // 処理したいものだけ登録してください。
   // 処理関数は、sensor_???.cpp の解説を参照してください。
   	// ARIA の処理関数
	  twelite.on([](const ParsedAriaPacket& packet){ on_pkt_ARIA(packet); });
		// PAL_AMB の処理関数
	  twelite.on([](const ParsedPalAmbPacket& packet){ on_pkt_PAL_AMB(packet); });
   	// CUE(デフォルト設定) の処理関数
     twelite.on([](const ParsedCuePacket& packet){ on_pkt_CUE(packet); });
   	// PAL_MOT,CUE(MOT設定) の処理関数
     twelite.on([](const ParsedPalMotPacket& packet){ on_pkt_PAL_MOT(packet); });
   
   // twelite の動作開始 (チャネルとアプリケーションIDの設定も行う)
     twelite.begin(18, 0x67720102);      // 周波数チャネル・アプリケーションIDの設定
   ```

### loop()
`MWings.update()`を呼び出し、TWELITE からの UART メッセージを処理します。

このサンプルでは WiFi の接続されているときに、通常の処理を行うようになっています。`MWings.update()`を呼び出すことで、TWELITE に接続されているシリアルポートからデータを呼び出します。TWELITE から一まとまりのデータを受け取った時点で、その内容を解釈し、無線アプリケーションに応じたコールバック関数を呼び出します。

```C++
	if (WiFi.status() == WL_CONNECTED) {
		// Wings シリアルパーサーの処理を行う。
		//   シリアルポートから入力データを読み出し、パケット系列が確定した時点で、
		//   事前に登録したコールバック関数(on_pkt_ARIA()など)を呼び出す。
		twelite.update();

		// LEDを消灯する
		digitalWrite(BLUE_LED, HIGH);
	}
	else {
		// WIFI 接続できていない状態
    ...
 		ESP.restart();
	}
```



## 解説-MWUtils

このライブラリはM5Stack(液晶付きESP32)向けに開発した mwm5 ライブラリからの抜粋です。ここでは、シリアル出力を `<<`演算子を用いて記述する WSerクラスオブジェクトを用いています。

### WSerシリアル出力

MWUtils内のprintfライブラリを用いたシリアル出力用を行います。



以下の例では、シリアルポート `Serial` に `Hello!`(CRLF改行) を出力します。`WSer`は`Serial`に対応します。`WSer2`は`Serial2`に対応します。

```C++
#include <MWutls.h>  // WSer シリアル出力
...
void somefunc() {
  WSer << "Hello!" << crlf;
}
```



printfライブラリを用いた出力を行うには `format()`を用います。`format()`に渡す第二引数以降は、最大６つまで指定できます。７つ以上指定するとコンパイル時のエラーになります。用法は`printf()`と同じです。

```C++
  WSer
  << "PKT/PAL_MOT: "
  << format("X=.4f Y=.4f Z=.4f", x_ave, y_ave, z_ave)
  << format(" [SER:%08X", d.u32SerID)
  << format(",SEQ:%05x", d.u16Seq)
  << format(",LID:%03d", d.u8LID)
  << format(",LQI:%03d", d.u8LQI) 
  << format(",VCC:%04d", d.u16Volt)
  << ']'
  << crlf;
```

