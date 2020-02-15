#define TIMER_DELAY 2000

#include "ESP8266WiFi.h"
#include "EEPROM.h"

char Ssid[32] = "";
char Password[32] = "";

int Port = 80;

bool IsConnectedWifi;

String inputString;
bool stringComplete;

int NumMode;
long LastTime;

WiFiServer Server(Port);

WiFiClient client;

void setup() {
	Init();
}


void loop() {
	SettingWifi();
	ReadSerialCommand();
	ReadClient();
}

void Init()
{
	Serial.begin(115200);
	IsConnectedWifi = false;
	NumMode = 0;

	loadCredentials();
	ConnectWifi();
	Server.begin();
	client = Server.available();
	LastTime = millis();
	
}

void loadCredentials() {
	EEPROM.begin(512);
	EEPROM.get(0, Ssid);
	EEPROM.get(0 + sizeof(Ssid), Password);
	char ok[2 + 1];
	EEPROM.get(0 + sizeof(Ssid) + sizeof(Password), ok);
	EEPROM.end();

	if (String(ok) != String("OK")) {
		Ssid[0] = 0;
		Password[0] = 0;
	}

	if (strlen(Ssid) > 0)
	{
	}
	else
	{
		Ssid[0] = 0;
		Password[0] = 0;
	}
}

void saveCredentials() {
	EEPROM.begin(512);
	EEPROM.put(0, Ssid);
	EEPROM.put(0 + sizeof(Ssid), Password);
	char ok[2 + 1] = "OK";
	EEPROM.put(0 + sizeof(Ssid) + sizeof(Password), ok);
	EEPROM.commit();
	EEPROM.end();
}

void clearCredentials() {
	char Ssid_d[32] = "";
	char Password_d[32] = "";
	EEPROM.begin(512);
	EEPROM.put(0, Ssid_d);
	EEPROM.put(0 + sizeof(Ssid_d), Password_d);
	char ok[2 + 1] = "";
	EEPROM.put(0 + sizeof(Ssid_d) + sizeof(Password_d), ok);
	EEPROM.commit();
	EEPROM.end();
	for (byte index = 0; index < 32; index++)
	{
		Ssid[index] = 0;
		Password[index] = 0;
	}
}

bool ConnectWifi()
{
	WiFi.disconnect();
	WiFi.mode(WIFI_STA);
	WiFi.begin(Ssid, Password);
	return CheckConnectWifi();
}

bool CheckConnectWifi()
{
	if (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		IsConnectedWifi = false;
		return false;
	}
	else
	{
		IsConnectedWifi = true;
		return true;
	}
}

void ReadSerialCommand()
{
	while (Serial.available())
	{
		char inChar = (char)Serial.read();

		if (IsConnectedWifi)
		{
			client.write(inChar);
		}

		if (inChar == '\n')
		{
			stringComplete = true;
			break;
		}

		if (inChar != '\r' && inChar != '\n')
		{
			inputString += inChar;
		}
	}

	if (!stringComplete)
		return;

	String message = inputString.substring(0, 4);



	if (message == "SSID")
	{
		inputString.substring(5).toCharArray(Ssid, 32);
		NumMode = 2;
	}

	if (message == "PSWD")
	{
		inputString.substring(5).toCharArray(Password, 32);
		NumMode = 4;
	}

	inputString = "";
	stringComplete = false;
}

void WaitSendIp()
{
	if (CheckConnectWifi())
	{
		Serial.print("ESPIP:");
		Serial.println(WiFi.localIP());
		NumMode = 6;
		saveCredentials();
	}
	else
	{
		loadCredentials();
		ConnectWifi();
		NumMode = 7;
	}
}

bool WaitMillis()
{
	if (millis() - LastTime > TIMER_DELAY)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SettingWifi()
{
	switch (NumMode)
	{
	case 0:
		if (WaitMillis())
		{
			if (CheckConnectWifi())
			{
				Serial.print("ESPIP:");
				Serial.println(WiFi.localIP());
				Serial.println("SAVEIP");
			}
		}
		else
		{
			break;
		}
		Serial.println("gSsid");
		Serial.println("gSsid");
		NumMode = 1;
		break;
	case 2:
		Serial.println("gPswd");
		NumMode = 3;
		break;
	case 4:
		if (strlen(Ssid) > 0)
		{
			ConnectWifi();
			LastTime = millis();
			NumMode = 5;
		}
		break;
	case 5:
		if (WaitMillis())
		{
			WaitSendIp();
		}
		break;
	case 6:
		Serial.println("SAVEIP");
		NumMode = 8;
		break;
	case 7:
		if (WaitMillis())
		{
			if (CheckConnectWifi())
			{
				Serial.print("ESPIP:");
				Serial.println(WiFi.localIP());
				Serial.println("SAVEIP");
			}
			NumMode = 8;
		}
	case 8:
		if (CheckConnectWifi())
		{
		}
		else
		{
			ConnectWifi();
		}

		break;
	default:
		break;
	}
}

void ReadClient()
{
	if (!IsConnectedWifi)
	{
		return;
	}

	if (client.connected()) {
		while (client.available() > 0) {
			Serial.write(client.read());
		}
	}
	else
	{
		client = Server.available();
	}
}