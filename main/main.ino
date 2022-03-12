#include <ArduinoJson.h>
#define CIRCULAR_BUFFER_INT_SAFE
#include <CircularBuffer.h>
#include <Servo.h>

constexpr int waterLevelPin = A0;
constexpr int buttonPin = 3;
constexpr int buttonLedPin = 2;
// Pin du robinet
constexpr int tapPin = 4;
Servo servo;

uint32_t lastEpochSentInfoSensor;

enum class State
{
  Idle,
  Filling,
  WaitToFill
} currentState = State::Idle;

struct Session
{
  int id;
  int startLevel;
  int endLevel;
  int quantity;
} currentSession;

void setup() {
  // Configuration des entrées sorties
  pinMode(waterLevelPin, INPUT);
  pinMode(tapPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Configuration sortie série
  Serial.begin(9600);
  Serial.setTimeout(500);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonLedPin, OUTPUT);
  //attachInterrupt(digitalPinToInterrupt(buttonPin), buttonChange, CHANGE);
  servo.attach(tapPin);

  lastEpochSentInfoSensor = millis();
}

/*vod buttonChange()
{
  const int status = digitalRead(buttonPin);
  digitalWrite(buttonLedPin, !status);
}*/

StaticJsonDocument<200> createInfoSensorJson(int waterLevelValue)
{
  StaticJsonDocument<100> doc;
  doc["type"] = "info_sensor";
  doc["params"]["water_level"] = waterLevelValue;
  doc["params"]["timestamp"] = millis();
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

void startSession(int id, int quantity)
{
  currentSession.startLevel = analogRead(waterLevelPin);
  currentSession.endLevel = currentSession.startLevel - quantity;
  currentSession.quantity = quantity;
  currentState = State::WaitToFill;
}

void closeSession()
{
  StaticJsonDocument<100> doc;
  doc["type"] = "session_closed";
  doc["id"] = currentSession.id;
  serializeJson(doc, Serial);
  Serial.println();
  
  currentState = State::Idle;
  closeTap();
}

void processQuery(const StaticJsonDocument<200> &doc)
{
  const String type = doc["type"];
  const int id = doc["id"];

  StaticJsonDocument<100> responseDoc;
  responseDoc["type"] = "ack";
  responseDoc["id"] = id;

  if (type == "debug_led") {
    const bool on = doc["params"]["status"];
    digitalWrite(LED_BUILTIN, on);
  }
  else if (type == "start_session") {
    const int quantity = doc["params"]["quantity"];
    startSession(id, quantity);
    responseDoc["params"]["start_level"] = currentSession.startLevel;
    responseDoc["params"]["end_level"] = currentSession.endLevel;
  }

  serializeJson(responseDoc, Serial);
  Serial.println();
}

void sendInfoSensor()
{
  // Récupere le niveau d'un capteur d'eau
  const int waterLevelValue = analogRead(waterLevelPin);

  StaticJsonDocument<100> doc = createInfoSensorJson(waterLevelValue);
  serializeJson(doc, Serial);
  Serial.println();
}

bool popQuery(struct Query &query)
{
  noInterrupts();
  const bool hasQuery = !queries.isEmpty();
  if (hasQuery) {
      query = queries.pop();
  }
  interrupts();

  return hasQuery;
}

void processOneQuery()
{
  Query query;
  const bool hasQuery = popQuery(query);

  if (hasQuery) {
      StaticJsonDocument<100> queryDoc;
      if (deserializeJson(queryDoc, query.rawValue) != DeserializationError::Ok) {
        Serial.println("{\"type\":\"json_error\"}");
      }
      else {
        processQuery(queryDoc);
      }
  }
}

void openTap()
{
  servo.write(60);
}

void closeTap()
{
  servo.write(30);
}

void listenButton()
{
  const int pressed = !digitalRead(buttonPin);

  const bool buttonLedOn = (currentState != State::Idle) && pressed;
  digitalWrite(buttonLedPin, buttonLedOn); 

  switch (pressed) {
    case false:
      switch (currentState) {
        case State::Filling:
          closeTap();
          currentState = State::WaitToFill;
          break;
      }
      break;
    case true:
      switch (currentState) {
        case State::WaitToFill:
          openTap();
          currentState = State::Filling;
          break;
      }
      break;
  }
}

void updateCurrentSession()
{
  if (currentState == State::Filling) {
    const int waterLevel = analogRead(waterLevelPin);
    const bool passedEnd = (waterLevel <= currentSession.endLevel);
    if (passedEnd) {
      closeSession();
    }
  }
}

void loop() {
  delay(100);

  const uint32_t elapsedTimeSinceLastSentInfoSensor = (millis() - lastEpochSentInfoSensor);
  if (elapsedTimeSinceLastSentInfoSensor > 500) {
    lastEpochSentInfoSensor = millis();
    sendInfoSensor();
  }

  processOneQuery();
  updateCurrentSession();
  listenButton();
}
