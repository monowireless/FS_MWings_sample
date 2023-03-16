// 共通定義
#include "common.h"

#ifndef DEBUG_UNUSE_AZURE
// インクルード
#include <AzureIoTHub.h> // Azure IoT Hub (MQTT PubSubClient)

void azureCallback(String s)
{
	Serial.print("azure Message arrived [");
	// Serial.print(s);
	Serial.println("] ");
}

String address(IPAddress ip)
{
	String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
	return ipStr;
}
#endif