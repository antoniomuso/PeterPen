#include <ESP8266WiFi.h>

WiFiClient client;
const char* host = "192.168.43.242";
bool isConnected = false;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.begin("HUAWEI P20", "1234abcd");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  if (client.connect(host, 8080))
  {
    isConnected = true;
  }
  else
  {
    isConnected = false;
  }
}

void loop() {
  if (isConnected) {
    client.print("Lesbico!!\n");  
  }
  delay(1000);
  
}