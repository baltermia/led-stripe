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
const char ssid[] = "";
const char pass[] = "";

int keyIndex = 0;

WiFiServer server(80);
// -----------------------------------------

void setup()
{
  Serial.begin(115200);
  
  while (! Serial) ;

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

  if (!isnan(tmp) && tmp != temperature)
  {
    temperature = tmp;
  }
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
          PrintHtmlToClient(client);
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
      if (currentLine.endsWith("GET /BRIGHTNESS/50")) {
        Serial.println("WebServer: BRIGHTNESS");
      }
    }
  }  
  
  client.stop();
}

// -- WiFi ---------------------------------
void PrintWifiStatus() 
{
  Serial.println("SSID: " + String(WiFi.SSID()));

  Serial.println("IP Address: " + WiFi.localIP());
}

void ConnectWiFi() 
{
  int status = WL_IDLE_STATUS;

  Serial.print("Attempting to connect to " + String(ssid));
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(500);
  }
  Serial.println();
}

void PrintHtmlToClient(WiFiClient client)
{
  client.println(
    "HTTP/1.1 200 OK\r\n"
    "Content-type:text/html\r\n"
    "\r\n"
    "<!DOCTYPE html>\n"
    "<title>LED-Strip Controller Page</title>\n"
    "</head>\n"
    "<body>\n"
      "<h1>Welcome to the LED-Strip Controller!</h1>\n"
      "Click <a href=\"/OFF\">here</a> to turn leds off<br>\n"
      "Click <a href=\"/OFF\">here</a> to turn leds off<br>\n"
      "<form onSubmit=\"return sendValue(\"BRIGHTNESS\", \"brightness\")\">"
        "<label for=\"brightness\">Brightness (between 0 and 100):</label>"
        "<input type=\"number\" id=\"brightness\" name=\"brightness\" min=\"0\" max=\"100\">"
      "</form>"
  );

  client.println("Current brightness: " + String(brightness));
  client.println("\nCurrent Temperature: " + String(temperature));

  client.println(
      "<script type=\"text/javascript\">"
        "function sendValue(url, id) {"
          "$.get(url + \"/\" + document.getElementById(id).value + \"/\""
        "}"
      "</script>"
    "</body>"
  );
}
// -----------------------------------------
