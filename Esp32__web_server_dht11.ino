#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <Arduino_JSON.h>
#include "SPIFFS.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

const char* ssid = "MALUPONTON";
const char* password = "Paulette20";

#define DHTPIN 27

#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);
AsyncEventSource events("/events");
JSONVar readings;

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;


void initSPIFFS() {
 if (!SPIFFS.begin(true)) {
 Serial.println("An error has occurred while mounting SPIFFS");
 }
 else{
 Serial.println("SPIFFS mounted successfully");
 }
}

String getSensorReadings(){
 readings["temperature"] = String(dht.readTemperature());
 readings["humidity"] = String(dht.readHumidity());
 String jsonString = JSON.stringify(readings);
 return jsonString;
}

void setup() {
  // put your setup code here, to run once:
// Serial port for debugging purposes
  Serial.begin(115200);
  initSPIFFS();
  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Route to load style.css file
   server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(SPIFFS, "/styles.css", "text/css");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
 String json = getSensorReadings();
 request->send(200, "application/json", json);
 json = String();
 });
 events.onConnect([](AsyncEventSourceClient *client){
 if(client->lastId()){
 Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
 }
 // send event with message "hello!", id current millis
 // and set reconnect delay to 1 second
 client->send("hello!", NULL, millis(), 10000);
 });
 server.addHandler(&events);
 // Start server
 server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
if ((millis() - lastTime) > timerDelay) {
 // Send Events to the client with the Sensor Readings Every 30 seconds
 events.send("ping",NULL,millis());
 events.send(getSensorReadings().c_str(),"new_readings" ,millis());
 lastTime = millis();
 }
}
