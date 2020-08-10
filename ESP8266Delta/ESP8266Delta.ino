#define TIMER_DELAY 2000

#include "ESP8266WiFi.h"
#include "EEPROM.h"

char Ssid[32] = "";
char Password[32] = "";

//int Port = 80;
int Port = 8844;

bool IsConnectedWifi;
bool IsSSID;
bool IsPSWD;
bool IsWaitConnect;

String inputString;
bool stringComplete;

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
	//delayMicroseconds(1);
}

void Init()
{
	delay(100);
	Serial.begin(115200);
	IsConnectedWifi = false;
	IsSSID = false;
	IsPSWD = false;
	IsWaitConnect = false;

	delay(100);

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
		IsSSID = true;
		Serial.println("Ok");
	}

	if (message == "PSWD")
	{
		inputString.substring(5).toCharArray(Password, 32);
		IsPSWD = true;
		Serial.println("Ok");
	}

	if (message == "gSSI")
	{
		Serial.print("Ssidfb:");
		Serial.println(Ssid);
	}

	if (message == "gPSW")
	{
		Serial.print("Pswdfb:");
		Serial.println(Password);
	}

	if (message == "ESIP")
	{
		if (IsConnectedWifi)
		{
			Serial.print("ESPIP:");
			Serial.println(WiFi.localIP());
		}	
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
		//delayMicroseconds(1);
		//Serial.println("Wificon");
		saveCredentials();
	}
	else
	{
		Serial.println("cncnnww");
		//delayMicroseconds(1);
		loadCredentials();
		//ConnectWifi();
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
	if (IsSSID && IsPSWD)
	{
		ConnectWifi();
		IsWaitConnect = true;
		IsSSID = false;
		IsPSWD = false;
		LastTime = millis();
	}
	
	if (IsWaitConnect)
	{
		if (WaitMillis())
		{
			WaitSendIp();
			IsWaitConnect = false;
		}
	}

	if (!IsWaitConnect)
	{
		if (!IsConnectedWifi)
		{
			ConnectWifi();
		}
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