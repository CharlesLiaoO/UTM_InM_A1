#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

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
  // String GAS_ID = "AKfycbyv7q2daZfMTkITgr2dzcca0-zToiQTCGWBKE8CNBTmYeesVczupLXkBJN6N3TbzVY2"; // Spreadsheet script ID AFTER DEPLOYMENT COMPLETE
  String GAS_ID = "1xKWhk7v38ztxQOtiykwv0SKDxTNLcHH7DCqhaj_zsY1A2kCkrETrlXcB"; // Spreadsheet script ID AFTER DEPLOYMENT COMPLETE
  String host = "script.googleusercontent.com"; // Google Apps Script web app host
  int httpsPort = 443; // HTTPS port

  Serial.println("==========");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Connect to Google Sheets host
  if (!client.connect(host, httpsPort)) {
    stopLoop("Connection failed!");
    return;
  }

  // Construct the URL with all variables as query parameters
  String string_t1 = String(tem);
  String string_h1 = String(hum);

  // String URL = "/macros/s/AKfycbyv7q2daZfMTkITgr2dzcca0-zToiQTCGWBKE8CNBTmYeesVczupLXkBJN6N3TbzVY2/exec?";  // Tutorial
  // https://script.google.com/macros/s/AKfycbwpfpPkXKb2kFAE0_lDingrMaw9sSZOgK9LH_ZZA12mY6gkqkbq60cOQk5fG6of-Lir-Q/exec  Mine, copy from App Script -> Manage deployments -> Web app
  String URL = "macros/s/AKfycbwpfpPkXKb2kFAE0_lDingrMaw9sSZOgK9LH_ZZA12mY6gkqkbq60cOQk5fG6of-Lir-Q/exec?";
  String total_url = URL + "tem=" + string_t1 + "&hum=" + string_h1;
  Serial.print("Requesting URL: ");
  Serial.println(total_url);

  // Send HTTP GET request to Google Apps Script web app
  client.print(String("GET ") + total_url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: BuildfailureDetectorESP8266\r\n" +
                "Connection: close\r\n\r\n");

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
