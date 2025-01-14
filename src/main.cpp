#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Ref:
//   [HTTP GET, works] https://www.youtube.com/watch?v=UF6IodGvq2I;
//   [HTTP POST, not specify httpClient lib，but script simple] https://www.youtube.com/watch?v=UF6IodGvq2I

// const char* ssid = "R1216_2.4GHz";
// const char* password = "r121612321";
const char* ssid = "lch-k20p";
const char* password = "lch1234567812";

const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbzX8WV7dugsY3Q9sYa1gqC_5jFHZhtaAvO_fXZ85c4KGmFoF8A2cDqev6hUdnkAzEZH5w"; // Spreadsheet script (Deployment) ID
WiFiClientSecure client;

const int pin_led = 2;  // built-in blue led
const int pin_led_alm = 12;  // alm led

#define DHTPIN 14      // Define the data pin connected to D6
#define DHTTYPE DHT22  // Define the DHT sensor type
DHT dht(DHTPIN, DHTTYPE);

int t_cloud_intv = 0.1 * 60 * 1000;  // Interval of 1 minute (1 minute * 60 seconds * 1000 milliseconds)

bool bStopLoop = false;
void stopLoop(const char * msg=0) {
  bStopLoop = true;
  if (msg)
    Serial.println(msg);
  Serial.println("Stop Loop");
  digitalWrite(pin_led, 0);
}

void sendData(float tem, float hum) {
  WiFiClientSecure client;
  client.setInsecure();

  Serial.println("==========");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Connect to Google Sheets host
  if (!client.connect(host, httpsPort)) {
    stopLoop("Connection failed!");
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?";
  String urlWithPars = url + "tem=" + String(tem) + "&hum=" + String(hum);
  if (0)  // not for realtime notification
    urlWithPars += "&alm=true";

  Serial.print("Requesting URL: ");
  Serial.println(urlWithPars);
  // Send HTTP GET request to Google Apps Script web app
  client.print(String("GET ") + urlWithPars + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: Nothing\r\n" +
                "Connection: close\r\n\r\n");

  Serial.println("Request sent");
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(pin_led, OUTPUT);
    digitalWrite(pin_led, 0);

    pinMode(pin_led_alm, OUTPUT);

    dht.begin();
    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WIFI");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to WiFi");

    digitalWrite(pin_led, 1);
}

void loop() {
  if (bStopLoop) {
    delay(10);
    return;
  }

  delay(2000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  // static float humidity;  // test code
  // static float temperature;
  // humidity += 0.1;
  // temperature += 0.1;

  if (isnan(humidity) || isnan(temperature)) {
    stopLoop("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Humidity=%.1f%% Temperature=%.1f°C\n", humidity, temperature);
  }

  if (temperature > 39) {
    digitalWrite(pin_led_alm, 1);
    Serial.println("Alm");
  } else {
    digitalWrite(pin_led_alm, 0);
  }

  static int t_cloud_b = 0;
  int t_cloud = millis();
  if (t_cloud - t_cloud_b >= t_cloud_intv) {
    t_cloud_b = t_cloud;
    sendData(temperature, humidity);
  }
}
