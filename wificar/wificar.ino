#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char *ssid = "<network>";
const char *password = "<password>";

bool ledState = 0;
const int ledPin = 2;

const byte L298N_BACK_MOTOR_FOWARD = D2;
const byte L298N_BACK_MOTOR_BACKWARDS = D4;
const byte L298N_FRONT_MOTOR_LEFT = D3;
const byte L298N_FRONT_MOTOR_RIGHT = D5;

bool BACK_MOTOR_FOWARD_STATE = 0;
bool BACK_MOTOR_BACKWARDS_STATE = 0;
bool FRONT_MOTOR_LEFT_STATE = 0;
bool FRONT_MOTOR_RIGHT_STATE = 0;

void LogState()
{
  String log1 = BACK_MOTOR_FOWARD_STATE ? "1" : "0";
  String log2 = BACK_MOTOR_BACKWARDS_STATE ? "1" : "0";
  String log3 = FRONT_MOTOR_LEFT_STATE ? "1" : "0";
  String log4 = FRONT_MOTOR_RIGHT_STATE ? "1" : "0";
  String log5 = ledState ? "1" : "0";
  String log = "Foward: " + log1 + "\nBackwards: " + log2 + "\nLeft: " + log3 + "\nRight: " + log4 + "\nLed: " + log5;
  Serial.println(log + "\n");
}

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients()
{
  ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, (char *)data);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    const char *final_direction = doc["final_direction"];
    Serial.print(final_direction);
    Serial.print("\n");

    if (strcmp(final_direction, "toggle") == 0)
    {
      ledState = !ledState;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Foward") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 1;
      BACK_MOTOR_BACKWARDS_STATE = 0;
      FRONT_MOTOR_LEFT_STATE = 0;
      FRONT_MOTOR_RIGHT_STATE = 0;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Foward_Left") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 1;
      BACK_MOTOR_BACKWARDS_STATE = 0;
      FRONT_MOTOR_LEFT_STATE = 1;
      FRONT_MOTOR_RIGHT_STATE = 0;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Foward_Right") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 1;
      BACK_MOTOR_BACKWARDS_STATE = 0;
      FRONT_MOTOR_LEFT_STATE = 0;
      FRONT_MOTOR_RIGHT_STATE = 1;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Backwards") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 0;
      BACK_MOTOR_BACKWARDS_STATE = 1;
      FRONT_MOTOR_LEFT_STATE = 0;
      FRONT_MOTOR_RIGHT_STATE = 0;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Backwards_Left") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 0;
      BACK_MOTOR_BACKWARDS_STATE = 1;
      FRONT_MOTOR_LEFT_STATE = 1;
      FRONT_MOTOR_RIGHT_STATE = 0;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Backwards_Right") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 0;
      BACK_MOTOR_BACKWARDS_STATE = 1;
      FRONT_MOTOR_LEFT_STATE = 0;
      FRONT_MOTOR_RIGHT_STATE = 1;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Right") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 0;
      BACK_MOTOR_BACKWARDS_STATE = 0;
      FRONT_MOTOR_LEFT_STATE = 0;
      FRONT_MOTOR_RIGHT_STATE = 1;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Left") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 0;
      BACK_MOTOR_BACKWARDS_STATE = 0;
      FRONT_MOTOR_LEFT_STATE = 1;
      FRONT_MOTOR_RIGHT_STATE = 0;
      LogState();
      notifyClients();
    }
    else if (strcmp(final_direction, "Stop") == 0)
    {
      BACK_MOTOR_FOWARD_STATE = 0;
      BACK_MOTOR_BACKWARDS_STATE = 0;
      FRONT_MOTOR_LEFT_STATE = 0;
      FRONT_MOTOR_RIGHT_STATE = 0;
      LogState();
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String &var)
{
  Serial.println(var);
  if (var == "STATE")
  {
    if (ledState)
    {
      return "ON";
    }
    else
    {
      return "OFF";
    }
  }
  return String();
}

void UpdateCarState()
{
  digitalWrite(ledPin, ledState);

  if (FRONT_MOTOR_LEFT_STATE == 1)
    digitalWrite(L298N_FRONT_MOTOR_LEFT, HIGH);
  else if (FRONT_MOTOR_LEFT_STATE == 0)
    digitalWrite(L298N_FRONT_MOTOR_LEFT, LOW);

  if (FRONT_MOTOR_RIGHT_STATE == 1)
    digitalWrite(L298N_FRONT_MOTOR_RIGHT, HIGH);
  else if (FRONT_MOTOR_RIGHT_STATE == 0)
    digitalWrite(L298N_FRONT_MOTOR_RIGHT, LOW);

  if (BACK_MOTOR_FOWARD_STATE == 1)
    digitalWrite(L298N_BACK_MOTOR_FOWARD, HIGH);
  else if (BACK_MOTOR_FOWARD_STATE == 0)
    digitalWrite(L298N_BACK_MOTOR_FOWARD, LOW);

  if (BACK_MOTOR_BACKWARDS_STATE == 1)
    digitalWrite(L298N_BACK_MOTOR_BACKWARDS, HIGH);
  else if (BACK_MOTOR_BACKWARDS_STATE == 0)
    digitalWrite(L298N_BACK_MOTOR_BACKWARDS, LOW);
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(L298N_BACK_MOTOR_FOWARD, OUTPUT);
  pinMode(L298N_BACK_MOTOR_BACKWARDS, OUTPUT);
  pinMode(L298N_FRONT_MOTOR_LEFT, OUTPUT);
  pinMode(L298N_FRONT_MOTOR_RIGHT, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(L298N_BACK_MOTOR_FOWARD, LOW);
  digitalWrite(L298N_BACK_MOTOR_BACKWARDS, LOW);
  digitalWrite(L298N_FRONT_MOTOR_LEFT, LOW);
  digitalWrite(L298N_FRONT_MOTOR_RIGHT, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", "Wifi Car by Berto", processor); });

  // Start server
  server.begin();
}

void loop()
{
  ws.cleanupClients();
  UpdateCarState();
}
