// PAL_MOT 形式のパケットの処理を行う
//
// 無線モジュール子機側(TWELITE CUEなど)は１パケットに１６サンプル格納して送信する。
// 子機側を連続的な送信モードに設定しておいたうえで、親機側では連続パケットを連結して
// 一定数のサンプル（本例では64）が得られた時点で DFT(FFT) の処理を行う。
// 本例では、周波数軸のデータに対してピークを上位３つ探索する。

// 共通定義
#include "common.h"

// C++標準ライブラリ
#include <memory>
#include <complex>

// システムライブラリ
#ifndef DEBUG_UNUSE_AZURE
#include <AzureIoTHub.h>  // Azure IoT Hub (MQTT PubSubClient)
#endif

// MWライブラリ
#include <MWings.h>  // TWELITE Wings シリアルパーサー
#include <MWutls.h>  // WSer シリアル出力

// 本例で使用するコード
#include "pkt_sequence.hpp"    // パケットの連続処理
#include "otfft.hpp" // DFT(FFT)処理

// パケットの連続チェックと格納クラス
// (スマートポインタ std::unique_ptr<> にて必要になってから実体を生成)
struct XYZ { REAL data[3]; };
using PSD = pkt_seqence_data<XYZ, 64>;
std::unique_ptr<PSD> psd; // 3軸

// DFT(FFT) 用の計算バッファ
static const uint32_t SIZE_FFT = 64; // FFTサンプル数(2のべき乗なので 128, 256 など)
    // サンプル数が多ければ解像度が高くなる半面、無線パケットの伝達が
    // 連続的に成功する必要がある。PAL_MOT モードでは１無線パケット
    // に１６サンプル格納される。
    // 成功率を上げるには、
    // ・受信機を近づける ・再送回数を増やす ・安定するチャネルを選択する
static std::complex<REAL> vfft[SIZE_FFT];  // input/output buffer
static std::complex<REAL> wfft[SIZE_FFT];  // work buffer
static REAL vfft_abs[3][SIZE_FFT / 2];     // FFT result (magnitude)

// ピーク判定（計算結果処理例）用のバッファ
static int pks_idx[3][3];    // vfft_abs[] のピークインデックス格納用
static REAL pks_value[3][3]; // ピーク値の格納用

// DFT(FFT) 実行とピークの判定
void do_FFT(const ParsedPalMotPacket& packet) {
  uint32_t t_s = micros(); // 計算時間の計測のため現在の[usec]を取得する
  // perform FFT operation (XYZ-AXIS)
  for(unsigned axis = 0; axis < 3; axis++) {
    for (unsigned i = 0; i < SIZE_FFT; i++) {
      vfft[i] = { psd->get_value(i).data[axis], 0.};
                                            // 解析対象の波形を格納
      wfft[i] = {};                         // ワークエリアは初期化
    }
    OTFFT::fft(SIZE_FFT, vfft, wfft);        // FFT の実行
    OTFFT::abs(SIZE_FFT, vfft, vfft_abs[axis]); // 絶対値の取得
  }
  uint32_t t_e = micros();  // 計算時間の計測のため終了時の[usec]を取得する

  WSer
  << format("[FFT] us=%d, SAMP=%d", t_e - t_s, SIZE_FFT) // 計算時間[usec]とサンプル数
  << crlf;

  // ピーク探索を行う
  for (int axis = 0; axis < 3; axis++) { // axis = 0..2 (X, Y, Z) の３軸を順次処理する
    // ピーク検索用のイテレータ
    auto it_beg = std::begin(vfft_abs[axis]); // 通常の配列なので it_beg は float* で &vfft_abs[0] になる。
    auto it_end = std::end(vfft_abs[axis]);   // it_end は配列の末尾の次 &v_fft_abs[SIZE_FFT] になる。

    // DC はピーク検索対象から外す（一時的に十分小さい値を入れる）
    auto v_dc = vfft_abs[axis][0]; // 後で戻せるように保存しておく
    vfft_abs[axis][0] = -1.0; // 十分小さい値を入れ検索対象から外す

    // XYZ 軸各々のピーク上位３つを検索する
    for(int i = 0; i <= 3; i++) {
      auto it = std::max_element(it_beg, it_end);
          // ピークのある配列位置(イテレータ≒ポインタ）を検索
          // std::max_element() はごく単純な線形探索のアルゴリズムであり、
          // これを３回処理して上位３つのピークを検索する非効率であるが、
          // OTFFT::fft() と比べば誤差レベルの計算量である。

      // 検索失敗時は it == it_end になる
      if (it != it_end) {
        pks_idx[axis][i] = it - it_beg; // ピークの配列インデックスを保存
        pks_value[axis][i] = *it; // ピーク値を後で戻せるように保存しておく
        *it = -1.0; // 十分小さい値を入れ続くピーク検索の対象から外す
      } else {
        pks_idx[axis][i] = -1;
        pks_value[axis][i] = -1.;
      }
    }

    // ピーク探索が終わったので一時的に変更した配列の値をもとに戻す
    vfft_abs[axis][0] = v_dc;
    for(int i = 0; i <= 3; i++) {
      if(pks_idx[axis][i] != -1) { vfft_abs[axis][pks_idx[axis][i]] = pks_value[axis][i]; }
    }

    // 結果の表示
    const char lbl_axis[] = "XYZ";
    WSer
    << "  " << lbl_axis[axis]
    << format(": DC=%.4f", vfft_abs[axis][0])
    << format(", MAX=%.4f/%02d, 2ND=%.4f/%02d, 3RD=%.4f/%02d"
        , pks_value[axis][0], pks_idx[axis][0]
        , pks_value[axis][1], pks_idx[axis][1]
        , pks_value[axis][2], pks_idx[axis][2])
    << crlf;
  }

#ifndef DEBUG_UNUSE_AZURE
  // Azure push
  Azure.connect();
  DataElement a = DataElement();

  a.setValue("Sensor", "PAL_MOT");

  {
    char strbuff[16];
    sprintf(strbuff, "%08x", packet.u32SourceSerialId);
    a.setValue("SID", strbuff);
  }

  a.setValue("ValueA", pks_value[0][0]); // X 軸ピーク値
  a.setValue("ValueB", pks_value[1][0]); // Y 軸ピーク値
  a.setValue("ValueC", pks_value[2][0]); // Z 軸ピーク値
  a.setValue("ValueD", (double)(0.));

  a.setValue("AttrA", pks_idx[0]); // X軸ピークのインデックス (e.g. サンプル周波数 * pks_idx[0] / SIZE_FFT)
  a.setValue("AttrB", pks_idx[1]); // Y軸ピークのインデックス
  a.setValue("AttrC", pks_idx[2]); // Z軸ピークのインデックス
  a.setValue("AttrD", (int)0);

  a.setValue("LQI", (int)packet.u8Lqi);
  a.setValue("VCC", (int)packet.u16SupplyVoltage);

  // Azure IoT Hub へプッシュ
  Azure.push(&a);
#endif
}

// MOTパケットの解釈ができたときに呼ばれる
void on_pkt_PAL_MOT(const ParsedPalMotPacket& packet) {
  REAL x_ave = 0, y_ave = 0, z_ave = 0; // このパケットでの XYZ 軸の平均値
  uint32_t t_now = millis(); // パケット到着タイムスタンプ

  // シリアルデータの解釈が終わったら青LEDを点灯する
  digitalWrite(BLUE_LED, LOW);

  // pkt_seqence_data オブジェクトを構築する
  if (!psd) psd.reset(new PSD());

  // パケット内のデータを処理する
  for (int i = 0; i < packet.u8SampleCount; i++) {
    // 連続するパケットから、連続データ列を構築する
    uint32_t n = psd->push(
        packet.u16SequenceNumber    // パケットの続き番号（これで連続かどうか判定する）
      , packet.u32SourceSerialId  // シリアル番号
      , t_now       // パケット受信時刻[ms]
      , { REAL(packet.i16SamplesX[i]) / REAL(1000.), REAL(packet.i16SamplesY[i]) / REAL(1000.), REAL(packet.i16SamplesZ[i]) / REAL(1000.) } // 3軸分のデータ struct XYZ; を投入
      );

    // psd->push() 処理で連続判定されると戻り値の n が報告される。
    //   パケットが抜けたり、タイムアウト時間が経過したりすると不連続として 0 に戻る
    // この値 n が DFT(FFT) のサンプル数を超えたところで、DFT(FFT)解析を行う。
    if (n >= SIZE_FFT) {
      do_FFT(packet);     // DFT(FFT)処理を行う
      psd->clear();  // バッファをクリアして新しい系列にする。
                     // クリアしない場合、１サンプルごとの区間を新しくすることも可能である。
    }

    // パケット内のサンプルの平均を計算 (パケット到達時のメッセージ用、DFT(FFT)の処理には使用しない)
    for(int i = 0; i < packet.u8SampleCount; i++) {
        x_ave += packet.i16SamplesX[i];
        y_ave += packet.i16SamplesY[i];
        z_ave += packet.i16SamplesZ[i];
    }
    REAL v_div = REAL(packet.u8SampleCount) * REAL(1000.);
    x_ave /= v_div;
    y_ave /= v_div;
    z_ave /= v_div;
  }

  // パケットの内容を表示する（確認用）
  WSer
  << "PKT/PAL_MOT: "
  << format("X=.4f Y=.4f Z=.4f", x_ave, y_ave, z_ave)
  << format(" [SER:%08X", packet.u32SourceSerialId)
  << format(",SEQ:%05x", packet.u16SequenceNumber)
  << format(",LID:%03d", packet.u8SourceLogicalId)
  << format(",LQI:%03d", packet.u8Lqi)
  << format(",VCC:%04d", packet.u16SupplyVoltage)
  << ']' << crlf;
}
