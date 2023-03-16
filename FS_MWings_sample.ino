// 共通定義
#include "common.h"
#include "x_my_connect.h"

#if defined(ESP8266)	 // for ESP8266
#include <ESP8266WiFi.h> // ESP8266 WiFi サポート
#elif defined(ESP32)	 // for ESP32
#include <WiFi.h>
#endif

#ifndef DEBUG_UNUSE_AZURE
#include <AzureIoTHub.h> // Azure IoT Hub (MQTT PubSubClient)
#endif

// for TWELITE
#include <MWings.h> // TWELITE Wings シリアルパーサー (new!)
#include <MWutls.h> // ユーティリティ (WSer, format()他)

// -------------
// 変数定義

// -------------
// クラスオブジェクト
MWings twelite;

// -------------
void setup()
{
// ----------
// シリアルポートの初期化
#if !defined(NO_SERIAL_RX_BUFFER) // ライブラリバージョンによっては setRxBufferSize()がない
	Serial.setRxBufferSize(512);  // 十分な大きさの UART 受信バッファを用意する
#else
# warning "*** no setRxBufferSize, maybe unstable when serial message is too long or too frequent. ***"
#endif
	Serial.begin(115200);

	Serial.println("");
	Serial.println("*** Boot WioNode");

	// ----------
	// IO周りの初期化
#if defined(BRD_WIONODE)
	pinMode(PORT_POWER, OUTPUT);
	digitalWrite(PORT_POWER, HIGH);
#endif

  // 確認用の LED
	pinMode(BLUE_LED, OUTPUT);

	// blink LED
	for (int i = 0; i < 10; i++)
	{
		digitalWrite(BLUE_LED, LOW);
		delay(50);
		digitalWrite(BLUE_LED, HIGH);
		delay(50);
	}

	// ----------
	// WiFi及びAzure周りの初期化
	// 固定IP化する為の処理(DHCP利用時は以下をコメントアウト)
	/*
	WiFi.config(IPAddress(192,168,0,12),IPAddress(192,168,0,1),IPAddress(255,255,255,0));
	Serial.println("---");
	Serial.print("Local IP  :");Serial.println(address(WiFi.localIP()));
	Serial.print("Gateway IP:");Serial.println(address(WiFi.gatewayIP()));
	Serial.print("SubnetMask:");Serial.println(address(WiFi.subnetMask()));
	Serial.print("macAddress:");Serial.println(WiFi.macAddress());
	//*/

	// WiFiクライアントモード設定
	WiFi.mode(WIFI_STA);
	//WiFi.printDiag(Serial); // WiFiを繋ぐ前に、WiFi状態をシリアルに出力
	WiFi.begin(MY_WIFI_SSID, MY_WIFI_KEY); // 2.4GHz帯のWiFiを指定すること(x_my_connect.h にSSIDと接続パスワードを記しておく：要取扱注意)

	// 接続が確立するまで、... を表示
	Serial.print("Connecting");
	int ct_wifi_try = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");

		if (ct_wifi_try++ > 50)
		{
			Serial.println("X");
			Serial.println("*** Failed to connect WiFi -- REBOOT ***");
			Serial.flush();
			delay(10); // just in case, wait for fulsh fully.

			ESP.restart();
		}
	}
	Serial.println();

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());

#ifndef DEBUG_UNUSE_AZURE
	// Azure のキー
	Azure.begin(MY_CONNECT_KEY);
	Azure.setCallback(azureCallback);

	Azure.connect();
	Serial.println("Azure.connect() finished");
#endif

	// -----------
	// シリアルオブジェクトの選択
#if defined(BRD_TWELITE_SPOT)
	Serial2.setRxBufferSize(1024);
	Serial2.begin(115200, SERIAL_8N1, 16, 17);
#endif

	// -----------
	// MWings シリアルパーサーの初期
#if defined(BRD_WIONODE)
	twelite.setup(Serial); // MWings の初期化
#elif defined(BRD_TWELITE_SPOT)
  twelite.debugUsing(Serial); // デバッグメッセージの出力設定
	twelite.setup(Serial2, PORT_TWE_RST, PORT_TWE_PGM);			  // MWings の初期化
#endif

	// ARIA の処理関数
  twelite.on([](const ParsedAriaPacket& packet){ on_pkt_ARIA(packet); });
	// PAL_AMB の処理関数
  twelite.on([](const ParsedPalAmbPacket& packet){ on_pkt_PAL_AMB(packet); });
	// CUE(デフォルト設定) の処理関数
  twelite.on([](const ParsedCuePacket& packet){ on_pkt_CUE(packet); });
	// PAL_MOT,CUE(MOT設定) の処理関数
  twelite.on([](const ParsedPalMotPacket& packet){ on_pkt_PAL_MOT(packet); });

  twelite.begin(18, 0x67720102);      // 周波数チャネル・アプリケーションIDの設定
}

void loop()
{
	if (WiFi.status() == WL_CONNECTED)
	{
		// Wings シリアルパーサーの処理を行う。
		//   シリアルポートから入力データを読み出し、パケット系列が確定した時点で、
		//   事前に登録したコールバック関数(on_pkt_ARIA()など)を呼び出す。
		twelite.update();

		// LEDを消灯する
		digitalWrite(BLUE_LED, HIGH);

#if defined(ESP32)
		static uint32_t u32Ct;

		if (millis() - u32Ct > 1000)
		{
			Serial.write('.');
			Serial2.write('.');
			Serial2.println(millis());
			u32Ct = millis();
		}

		while (Serial.available())
		{
			Serial2.write(char_t(Serial.read()));
		}
#endif
	}
	else
	{
		// WIFI 接続できていない状態
		Serial.print("*** Lose WiFi Connection -- REBOOT ***");
		Serial.println(WiFi.status());
		Serial.flush();
		delay(1000);

		ESP.restart();
	}
}
