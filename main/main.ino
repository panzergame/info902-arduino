#include <ArduinoJson.h>
#define CIRCULAR_BUFFER_INT_SAFE
#include <CircularBuffer.h>
constexpr int waterLevelPin = A0;

void setup() {
  // Configuration des entrées sorties
  pinMode(waterLevelPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Configuration sortie série
  Serial.begin(9600);
  Serial.setTimeout(500);
}

StaticJsonDocument<200> createInfoSensorJson(int waterLevelValue)
{
  StaticJsonDocument<200> doc;
  doc["type"] = "info_sensor";
  doc["params"]["water_level"] = waterLevelValue;
}

struct Query
{
  String rawValue;
};

CircularBuffer<Query, 5> queries;

void serialEvent()
{
  static String input;
  while (Serial.available()) {
    const char b = Serial.read();
    input += b;
    if (b == '\n') {
      Query query;
      query.rawValue = input;
      queries.push(query);
      input = "";
    }
  }
}

void processQuery(const StaticJsonDocument<200> &doc)
{
  const String type = doc["type"];
  const int id = doc["id"];

  StaticJsonDocument<200> responseDoc;
  responseDoc["type"] = "ack";
  responseDoc["id"] = id;

  if (type == "debug_led") {
    const bool on = doc["params"]["status"];
    digitalWrite(LED_BUILTIN, on);
  }

  serializeJson(responseDoc, Serial);
  Serial.println();
}

void loop() {
  delay(500);
  // Récupere le niveau d'un capteur d'eau
  const int waterLevelValue = analogRead(waterLevelPin);

  StaticJsonDocument<200> doc = createInfoSensorJson(waterLevelValue);
  serializeJson(doc, Serial);
  Serial.println();

  Query query;
  noInterrupts();
  const bool hasQuery = !queries.isEmpty();
  if (hasQuery) {
      query = queries.pop();
  }
  interrupts();

  if (hasQuery) {
      if (deserializeJson(doc, query.rawValue) != DeserializationError::Ok) {
        Serial.println("{\"type\":\"json_error\"}");
      }
      else {
        processQuery(doc);
      }
  }
}
