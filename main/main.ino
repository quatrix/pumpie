#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#define PUMP_ML_PER_MINUTE 60

#define BLYNK_TOKEN  "da620847b9b74d53802b9abef1390732"
#define BLYNK_PORT 8442

#define WIFI_SSID "evilnet"
#define WIFI_PASS "evil@me4ever"


SimpleTimer timer;

int pumps[] = {0, 0};

// 0 -> 4
// 1 -> 5
int pumpsMap[] = {4, 5};

bool shouldRunPumps = false;
int timeOfEvent = 0;

IPAddress ip(192,168,1,101);  //Node static IP
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.config(ip, gateway, subnet, dns);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);    // Initialize serial communications
    setupWiFi();

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);

    Blynk.config(BLYNK_TOKEN);
    timer.setInterval(250, sendStatus);

}

void sendStatus() {
    Blynk.virtualWrite(V3, pumps[0]);
    Blynk.virtualWrite(V5, pumps[1]);
}
BLYNK_WRITE(V0)
{
    pumps[0] = param.asInt();
    Blynk.virtualWrite(V3, pumps[0]);
}

BLYNK_WRITE(V1)
{
    pumps[1] = param.asInt();
    Blynk.virtualWrite(V5, pumps[1]);
}

BLYNK_WRITE(V2)
{
    shouldRunPumps = true;
    timeOfEvent = millis();
}

void runPumps() {
    for (int i = 0; i < 2; i++) {
        digitalWrite(pumpsMap[i], LOW);
        delay(PUMP_ML_PER_MINUTE / 60 * pumps[i] * 1000);
        digitalWrite(pumpsMap[i], HIGH);
    }
}

void loop() {
    Blynk.run();
    timer.run();

    if (millis() - timeOfEvent > 500 && shouldRunPumps) {
        runPumps();
        shouldRunPumps = false;
    }
}
