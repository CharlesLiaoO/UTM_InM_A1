#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// Ref:
//   [HTTP GET, works] https://www.youtube.com/watch?v=UF6IodGvq2I;
//   [HTTP POST, not specify httpClient lib，but script simple] https://www.youtube.com/watch?v=UF6IodGvq2I

const char* ssid = "R1216_2.4GHz";
const char* password = "r121612321";
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
#define DHTPIN D6      // Define the data pin connected to D6
#define DHTTYPE DHT11  // Define the DHT sensor type
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;  // Stores the previous time in milliseconds
unsigned long interval = 0.1 * 60 * 1000;  // Interval of 1 minute (1 minute * 60 seconds * 1000 milliseconds)

bool bStopLoop = false;
void stopLoop(const char * msg=0) {
  bStopLoop = true;
  if (msg)
    Serial.println(msg);
  Serial.println("Stop Loop");
}

void sendData(float tem, float hum) {
  WiFiClientSecure client;

  client.setInsecure();
  String GAS_ID = "AKfycbzX8WV7dugsY3Q9sYa1gqC_5jFHZhtaAvO_fXZ85c4KGmFoF8A2cDqev6hUdnkAzEZH5w"; // Spreadsheet script (Deployment) ID

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
  Serial.print("Requesting URL: ");
  Serial.println(urlWithPars);

  // Send HTTP GET request to Google Apps Script web app
  client.print(String("GET ") + urlWithPars + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: Nothing\r\n" +
                "Connection: close\r\n\r\n");
  client.available();

  Serial.println("Request sent");
}

void setup() {
    Serial.begin(115200);
    Serial.println();

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
}

void loop() {
  if (bStopLoop) {
    delay(10);
    return;
  }

  delay(2000); // Delay for 2 seconds

  // float humidity = dht.readHumidity();
  // float temperature = dht.readTemperature();
  static float humidity;
  static float temperature;
  humidity += 0.1;
  temperature += 0.1;

  if (isnan(humidity) || isnan(temperature)) {
    stopLoop("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%\t");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
  }

  //calling function sendData for pushing to dheets
  if (millis() - previousMillis >= interval) {
    previousMillis = millis(); // Update the previous time - the time set every 1 minutes
    sendData(temperature, humidity); //copy variable that store the sensor value
  }
}
