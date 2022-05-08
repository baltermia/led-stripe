#include <Arduino.h>
#include <RotaryEncoder.h>
#include <DHT.h>
#include <WiFiNINA.h>
#include <FastLED.h>

// -- Brightness / Rotary-Sensor -----------
#define ROTARY1 2
#define ROTARY2 3

RotaryEncoder rotary(ROTARY1, ROTARY2, RotaryEncoder::LatchMode::TWO03);

// Range from 0 to 100
int brightness = 50;
// -----------------------------------------

// -- Temperature / DHT-Sensor -------------
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temperature;
float start = 22; 
// -----------------------------------------

// -- WebServer ----------------------------
const char ssid[] = "";
const char pass[] = "";

WiFiServer server(80);
// -----------------------------------------

// -- LED ----------------------------------
#define LEDPIN 5
#define PIXELS 60

CRGB leds[PIXELS];

static bool toggle = true;

enum Mode { Rainbow, Wave, Single, Temperature };
Mode mode = Rainbow;
CRGB color = CRGB::Black;
int waveSpeed = 10;
// -----------------------------------------

void setup()
{
  Serial.begin(115200);
  
  while (! Serial) ;

  ConnectWiFi();

  server.begin();
  PrintWifiStatus();

  dht.begin();

  start = dht.readTemperature();

  FastLED.addLeds<NEOPIXEL, LEDPIN>(leds, PIXELS);
}

void loop()
{
  RotarySensor();

  TemperatureSensor();

  WebServer();

  DoLedWork();
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

      const String get = "GET /";

      const static String modes[6] = {
        "WAVE-",
        "TEMPERATURE",
        "BRIGHTNESS-",
        "RAINBOW",
        "TOGGLE",
        "COLOR-"
      };

      String foundStr = "";
      String foundValue = "";

      for (const String mode : modes) {
        bool withValue = mode[mode.length() - 1] == '-';
        bool found = false;
        bool pastValue = false;
        for (int j = 0; j < currentLine.length(); j++)
        {
          char current = currentLine[j];
          if (pastValue){
              if (current == '/') {
                if (currentLine.length() > j + 1) {
                  break;
                }
                found = true;
                break;
              }
              foundValue += current;
          }  
          else if (j >= 5) {
            if (withValue && current == '-') {
                pastValue = true;
            }
            else if (current == '/') {
              if (currentLine.length() > j + 1) {
                  break;
              }
              found = true;
              break;
            }

            if (mode[j - 5] != current) {
              break;
            }
          } 
          else if (current != get[j])
          {
            break;
          }
        }
        if (found) {
          foundStr = mode;
          break;
        }
      }

      if (foundStr == "WAVE-") {
          mode = Wave;
          waveSpeed = foundValue.toInt();
      }
      else if (foundStr == "RAINBOW"){
          mode = Rainbow;
      }
      else if (foundStr == "COLOR-"){
          mode = Single;
          String hex = "0x" + foundValue;
          color = strtol(hex.c_str(), NULL, 0);
      }
      else if (foundStr == "TEMPERATURE"){
          mode = Temperature;
      }
      else if (foundStr == "TOGGLE"){
          toggle = !toggle;
      }
      else if (foundStr == "BRIGHTNESS-"){
          brightness = foundValue.toInt();
      }
    }
  }

  client.stop();
}

// -- WiFi ---------------------------------
void PrintWifiStatus() 
{
  Serial.println("SSID: " + String(WiFi.SSID()));

  Serial.print("IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.println();
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
    "<head>\n"
        "<script type='text/javascript'>\n"
            "function sendValue(url, id) {\n"
                "if (id) {\n"
                    "window.location.href = '/' + url + '-' + document.getElementById(id).value.replace('#', '') + '/';\n"
                "}\n"
                "else {\n"
                    "window.location.href = '/' + url + '/';\n"
                "}\n"
            "}\n"
        "</script>\n"
    "</head>\n"
    "<body>\n"
        "<h1>Welcome to the LED-Strip Controller!</h1>\n"

        "<h3>Toggle:</h3>\n"
        "<button onclick=\"sendValue('TOGGLE')\">Submit</button>\n"

        "<h3>Brightness (between 0 and 100):</h3>\n"
        "<input type='number' id='brightness' min='0' max='100' value='" + String(brightness) + "' />\n"
        "<button onclick=\"sendValue('BRIGHTNESS', 'brightness')\">Submit</button>\n"

        "<h3>Single Color:</h3>\n"
        "<input type='color' id='color' />\n"
        "<button onclick=\"sendValue('COLOR', 'color')\">Submit</button>\n"

        "<h3>Enable Rainbow:</h3>\n"
        "<button onclick=\"sendValue('RAINBOW')\">Submit</button>\n"

        "<h3>Wave + Speed:</h3>\n"
        "<input type='number' id='wave' min='0' max='100' value='" + String(waveSpeed) + "' />\n"
        "<button onclick=\"sendValue('WAVE', 'wave')\">Submit</button>\n"

        "<h3>Enable Temperature:</h3>\n"
        "Current Temperature: " + String(temperature) + "\n"
        "<button onclick=\"sendValue('TEMPERATURE')\">Submit</button>\n"
    "</body>\n"
  );
}
// -----------------------------------------

// -- LED ----------------------------------
void DoLedWork()
{
  if (!toggle) {
    ResetLeds();

    FastLED.show();

    return;
  }

  if (mode == Single) 
  {
    DoSingle();
  }
  else if (mode == Temperature)
  {
    DoTemperature();
  }
  else if (mode == Wave)
  {
    DoWave();
  }
  else if (mode == Rainbow)
  {
    DoRainbow();
  }

  FastLED.setBrightness(round(255 / 100 * brightness));

  FastLED.show();
}

void DoSingle()
{
  fill_solid(leds, PIXELS, color);
}

void DoTemperature()
{

}

void DoWave()
{
  fill_solid(leds, PIXELS, CHSV(beatsin8(waveSpeed), 255, 255));
}

void DoRainbow()
{
  fill_rainbow(leds, PIXELS, beat8(20, 255), 10);  
}

void ResetLeds()
{
  fill_solid(leds, PIXELS, CRGB::Black);
}
// -----------------------------------------