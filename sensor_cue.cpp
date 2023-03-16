// 共通定義
#include "common.h"

// システムライブラリ
#ifndef DEBUG_UNUSE_AZURE
#include <AzureIoTHub.h> // Azure IoT Hub (MQTT PubSubClient)
#endif
#include <MWings.h>      // TWELITE Wings シリアルパーサー

// CUEパケットの解釈ができたときに呼ばれる
void on_pkt_CUE(const ParsedCuePacket &packet)
{
	// シリアルデータの解釈が終わったら青LEDを点灯する
	digitalWrite(BLUE_LED, LOW);

    // パケットのデータ処理を行う
    double x_ave = 0, y_ave = 0, z_ave = 0;
    for(int i = 0; i < packet.u8SampleCount; i++) {
        x_ave += packet.i16SamplesX[i];
        y_ave += packet.i16SamplesY[i];
        z_ave += packet.i16SamplesZ[i];
    }
    x_ave /= packet.u8SampleCount * 1000.;
    y_ave /= packet.u8SampleCount * 1000.;
    z_ave /= packet.u8SampleCount * 1000.;

	// パケットの内容を表示する（確認用）
	Serial.print("PKT/CUE: ");

    Serial.print(" X = ");
    Serial.print(x_ave);
    Serial.print(" Y = ");
    Serial.print(y_ave);
    Serial.print(" Z = ");
    Serial.print(z_ave);

	Serial.print(" [SER:");
	Serial.print(packet.u32SourceSerialId, HEX);
	Serial.print(",SEQ:");
	Serial.print(packet.u16SequenceNumber, DEC);
	Serial.print(",LID:");
	Serial.print(packet.u8SourceLogicalId, DEC);
	Serial.print(",LQI:");
	Serial.print(packet.u8Lqi, DEC);
	Serial.print(",VCC:");
	Serial.print(packet.u16SupplyVoltage, DEC);
	Serial.print(",MAG:");
	Serial.print(packet.u8MagnetState, HEX);
	Serial.print(packet.bMagnetStateChanged ? '*' : ' ');
    Serial.print(",ACC:");
    for(int i = 0; i < packet.u8SampleCount; i++) {
        Serial.print('(');
        Serial.print(packet.i16SamplesX[i]);
        Serial.print(',');
        Serial.print(packet.i16SamplesY[i]);
        Serial.print(',');
        Serial.print(packet.i16SamplesZ[i]);
        Serial.print(')');
    }
    Serial.print("]");
	Serial.println("");

#ifndef DEBUG_UNUSE_AZURE
	// Azure push
	Azure.connect();
	DataElement a = DataElement();

	a.setValue("Sensor", "CUE");

	{
		char strbuff[16];
		sprintf(strbuff, "%08x", packet.u32SourceSerialId);
		a.setValue("SID", strbuff);
	}

	a.setValue("ValueA", x_ave);
	a.setValue("ValueB", y_ave);
	a.setValue("ValueC", z_ave);
	a.setValue("ValueD", (double)(0.));
	a.setValue("MAG", (int)(packet.u8MagnetState));

	a.setValue("LQI", (int)packet.u8Lqi);
	a.setValue("VCC", (int)packet.u16SupplyVoltage);

	// Azure IoT Hub へプッシュ
	Azure.push(&a);
#endif
}

// // ARIAパケットの解釈ができたときに呼ばれる
// void on_pkt_CUE(tsMWings_App_CUE &d)
// {
// 	// シリアルデータの解釈が終わったら青LEDを点灯する
// 	digitalWrite(BLUE_LED, LOW);

//     // パケットのデータ処理を行う
//     double x_ave = 0, y_ave = 0, z_ave = 0;
//     for(int i = 0; i < d.u8Ct; i++) {
//         x_ave += d.x[i];
//         y_ave += d.y[i];
//         z_ave += d.z[i];
//     }
//     x_ave /= d.u8Ct * 1000.;
//     y_ave /= d.u8Ct * 1000.;
//     z_ave /= d.u8Ct * 1000.;

// 	// パケットの内容を表示する（確認用）
// 	Serial.print("PKT/CUE: ");

//     Serial.print(" X = ");
//     Serial.print(x_ave);
//     Serial.print(" Y = ");
//     Serial.print(y_ave);
//     Serial.print(" Z = ");
//     Serial.print(z_ave);

// 	Serial.print(" [SER:");
// 	Serial.print(d.u32SerID, HEX);
// 	Serial.print(",SEQ:");
// 	Serial.print(d.u16Seq, DEC);
// 	Serial.print(",LID:");
// 	Serial.print(d.u8LID, DEC);
// 	Serial.print(",LQI:");
// 	Serial.print(d.u8LQI, DEC);
// 	Serial.print(",VCC:");
// 	Serial.print(d.u16Volt, DEC);
// 	Serial.print(",MAG:");
// 	Serial.print(d.u8Mag, HEX);
// 	Serial.print(d.bMagChanged ? '*' : ' ');
//     Serial.print(",ACC:");
//     for(int i = 0; i < d.u8Ct; i++) {
//         Serial.print('(');
//         Serial.print(d.x[i]);
//         Serial.print(',');
//         Serial.print(d.y[i]);
//         Serial.print(',');
//         Serial.print(d.z[i]);
//         Serial.print(')');
//     }
//     Serial.print("]");
// 	Serial.println("");

// #ifndef DEBUG_UNUSE_AZURE
// 	// Azure push
// 	Azure.connect();
// 	DataElement a = DataElement();

// 	a.setValue("Sensor", "CUE");

// 	{
// 		char strbuff[16];
// 		sprintf(strbuff, "%08x", d.u32SerID);
// 		a.setValue("SID", strbuff);
// 	}

// 	a.setValue("ValueA", x_ave);
// 	a.setValue("ValueB", y_ave);
// 	a.setValue("ValueC", z_ave);
// 	a.setValue("ValueD", (double)(0.));
// 	a.setValue("MAG", (int)(d.u8Mag));

// 	a.setValue("LQI", (int)d.u8LQI);
// 	a.setValue("VCC", (int)d.u16Volt);

// 	// Azure IoT Hub へプッシュ
// 	Azure.push(&a);
// #endif
// }
