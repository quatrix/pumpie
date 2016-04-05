#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "pumpie";
const char *password = "whereeveryougothereyouare";

ESP8266WebServer server(80);

int pumpsMap[] = {2, 14, 12, 13, 15};

void setup() {
    for (int i = 0; i < 5; i++) {
        pinMode(pumpsMap[i], OUTPUT);
        //digitalWrite(pumpsMap[i], LOW);
    }

    Serial.begin(115200);    // Initialize serial communications

    WiFi.softAP(ssid, password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.begin();
}

void handleRoot() {
    int pumpId, duration;
    int found = 0;

    for (uint8_t i=0; i<server.args(); i++){
          if (server.argName(i) == "pump") {
              pumpId = server.arg(i).toInt();
              found++;
          }
          
          if (server.argName(i) == "duration") {
              duration = server.arg(i).toInt();
              found++;
          }
    }

    if (found == 2) {
        runPump(pumpId, duration);
        server.send(200, "text/html", "OK\n");
    } else {
        server.send(500, "text/html", "ERROR\n");
    }

}

void runPump(int pumpId, int duration) {
    int pin = pumpsMap[pumpId];

    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
}

void loop() {
    server.handleClient();
}
