#include <Arduino.h>
#include <RotaryEncoder.h>
#include <DHT.h>
#include <WiFiNINA.h>

// -- Brightness / Rotary-Sensor -----------
#define ROTARY1 2
#define ROTARY2 3

RotaryEncoder rotary(ROTARY1, ROTARY2, RotaryEncoder::LatchMode::TWO03);

// Range from 0 to 100
static int brightness = 100;
// -----------------------------------------

// -- Temperature / DHT-Sensor -------------
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

static float temperature;

bool checkTemperature = true;
// -----------------------------------------

// -- WebServer ----------------------------
const char ssid[] = "username";
const char pass[] = "password";

int keyIndex = 0;

WiFiServer server(80);
// -----------------------------------------

void setup()
{
  Serial.begin(115200);
  
  while (! Serial) ;

  EnableWiFi();
  ConnectWiFi();

  server.begin();
  PrintWifiStatus();

  dht.begin();
}

void loop()
{
  RotarySensor();

  if (checkTemperature)
  {
    TemperatureSensor();
  }

  WebServer();
}

void RotarySensor() 
{
  static int pos = 0;
  rotary.tick();

  int newPos = rotary.getPosition();

  if (pos == newPos) 
  {
    return;
  }

  bool isAdd = (int)rotary.getDirection() == 1;

  if (isAdd && brightness < 100)
  {
    brightness++;
  }
  else if (!isAdd && brightness > 0) 
  {
    brightness--;
  }
   
  pos = newPos;
}

void TemperatureSensor() 
{
  float tmp = dht.readTemperature();

  if (isnan(tmp))
  {
    return;
  }
  
  if (tmp == temperature) 
  {
    return;
  }

  temperature = tmp;
}

void WebServer() 
{
  WiFiClient client = server.available();

  if (!client)
  {
    client.stop();
    return;
  }

  String currentLine = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      // Print whole requests to serial (DEBUG only)
      // Serial.write(c); 
      if (c == '\n') {
        if (currentLine.length() == 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          client.print("Click <a href=\"/ON\">here</a> to turn leds on<br>");
          client.print("Click <a href=\"/OFF\">here</a> to turn leds off<br>");

          client.println("Curren brightness: ");
          client.print(brightness);

          client.println("Curren temperature: ");
          client.print(temperature);

          client.println();
          break;
        }
        else {
          currentLine = "";
        }
      }
      else if (c != '\r') {
        currentLine += c;
        }

      if (currentLine.endsWith("GET /ON")) {
        Serial.println("WebServer: ON");
      }
      if (currentLine.endsWith("GET /OFF")) {
        Serial.println("WebServer: OFF");
      }
    }
  }  
  
  client.stop();
}

// -- WiFi ---------------------------------
void PrintWifiStatus() 
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void EnableWiFi() 
{
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
}

void ConnectWiFi() 
{
  int status = WL_IDLE_STATUS;

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 1 seconds for connection:
    delay(1000);
  }
}
// -----------------------------------------
