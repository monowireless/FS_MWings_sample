// 共通定義
#include "common.h"

// システムライブラリ
#ifndef DEBUG_UNUSE_AZURE
#include <AzureIoTHub.h> // Azure IoT Hub (MQTT PubSubClient)
#endif
#include <MWings.h>      // TWELITE Wings シリアルパーサー

// ARIAパケットの解釈ができたときに呼ばれる
void on_pkt_ARIA(const ParsedAriaPacket &packet)
{
	// シリアルデータの解釈が終わったら青LEDを点灯する
	digitalWrite(BLUE_LED, LOW);

	// パケットの内容を表示する（確認用）
	Serial.print("PKT/ARIA: ");

	Serial.print("T=");
	print_div100(packet.i16Temp100x);
	Serial.print("C ");

	Serial.print("H=");
	print_div100(packet.u16Humid100x);
	Serial.print("%");

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
	Serial.print(",TMP:");
	Serial.print(packet.i16Temp100x, DEC);
	Serial.print(",HMD:");
	Serial.print(packet.u16Humid100x, DEC);
	Serial.print(",MAG:");
	Serial.print(packet.u8MagnetState, HEX);
	Serial.print(packet.bMagnetStateChanged ? '*' : ' ');
	Serial.print("]");

	Serial.println("");

#ifndef DEBUG_UNUSE_AZURE
	// Azure push
	Azure.connect();
	DataElement a = DataElement();

	a.setValue("Sensor", "ARIA");

	{
		char strbuff[16];
		sprintf(strbuff, "%08x", packet.u32SourceSerialId);
		a.setValue("SID", strbuff);
	}

	a.setValue("ValueA", (double)(packet.i16Temp100x / 100.));
	a.setValue("ValueB", (double)(packet.u16Humid100x / 100.));
	a.setValue("ValueC", (double)(0.));
	a.setValue("ValueD", (double)(0.));
	a.setValue("MAG", (int)(packet.u8MagnetState));

	a.setValue("LQI", (int)packet.u8Lqi);
	a.setValue("VCC", (int)packet.u16SupplyVoltage);

	// Azure IoT Hub へプッシュ
	Azure.push(&a);
#endif
}

// // ARIAパケットの解釈ができたときに呼ばれる
// void on_pkt_ARIA(tsMWings_App_ARIA &d)
// {
// 	// シリアルデータの解釈が終わったら青LEDを点灯する
// 	digitalWrite(BLUE_LED, LOW);

// 	// パケットの内容を表示する（確認用）
// 	Serial.print("PKT/ARIA: ");

// 	Serial.print("T=");
// 	print_div100(d.i16Temp);
// 	Serial.print("C ");

// 	Serial.print("H=");
// 	print_div100(d.u16Humid);
// 	Serial.print("%");

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
// 	Serial.print(",TMP:");
// 	Serial.print(d.i16Temp, DEC);
// 	Serial.print(",HMD:");
// 	Serial.print(d.u16Humid, DEC);
// 	Serial.print(",MAG:");
// 	Serial.print(d.u8Mag, HEX);
// 	Serial.print(d.bMagChanged ? '*' : ' ');
// 	Serial.print("]");

// 	Serial.println("");

// #ifndef DEBUG_UNUSE_AZURE
// 	// Azure push
// 	Azure.connect();
// 	DataElement a = DataElement();

// 	a.setValue("Sensor", "ARIA");

// 	{
// 		char strbuff[16];
// 		sprintf(strbuff, "%08x", d.u32SerID);
// 		a.setValue("SID", strbuff);
// 	}

// 	a.setValue("ValueA", (double)(d.i16Temp / 100.));
// 	a.setValue("ValueB", (double)(d.u16Humid / 100.));
// 	a.setValue("ValueC", (double)(0.));
// 	a.setValue("ValueD", (double)(0.));
// 	a.setValue("MAG", (int)(d.u8Mag));

// 	a.setValue("LQI", (int)d.u8LQI);
// 	a.setValue("VCC", (int)d.u16Volt);

// 	// Azure IoT Hub へプッシュ
// 	Azure.push(&a);
// #endif
// }
