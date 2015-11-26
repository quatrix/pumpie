#include <ESP8266WiFi.h>

typedef struct {
    int id;
    int ml;
} pumpRequest;

pumpRequest parsedRequest;

const char WiFiAPPSK[] = "pump it up";
const String URLPrefix = "GET /pump/";

#define PUMP_ML_PER_MINUTE 60

WiFiServer server(80);

void setupWiFi() {
    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "HOTBOX__" + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i=0; i<AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);
    Serial.println("ready.");
}

void setupHardware() {
    Serial.begin(115200);    // Initialize serial communications

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
}

void setup() {
    setupHardware();
    setupWiFi();
    server.begin();
}


void resetRequest() {
    parsedRequest.id = 0;
    parsedRequest.ml = 0;
}

void parseRequest(String req) {
    Serial.println(req);

    resetRequest();

    if (req.indexOf(URLPrefix) == -1)
        return;

    if (req.length() == URLPrefix.length())
        return;
    
    req = req.substring(URLPrefix.length());
    req = req.substring(0, req.indexOf(' '));

    int slashIndex = req.indexOf('/');

    if (slashIndex == -1)
        return;

    parsedRequest.id = req.substring(0, slashIndex).toInt();
    parsedRequest.ml = req.substring(slashIndex+1).toInt();
}



void sendResponseToClient(WiFiClient client, String response) {
    client.flush();

    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n";
    s += response;

    client.print(s);
    client.flush();
    delay(1);
}

void runPump(int id, int ml) {
    digitalWrite(id, LOW);
    delay(PUMP_ML_PER_MINUTE / 60 * ml * 1000);
    digitalWrite(id, HIGH);
}

void loop() {
    WiFiClient client = server.available();

    if (!client) {
        return;
    }


    String req = client.readStringUntil('\r');
    parseRequest(req);

    client.flush();

    // Prepare the response. Start with the common header:
    // If we're setting the LED, print out a message saying we did
    if (parsedRequest.id != 0 && parsedRequest.ml != 0) {
        sendResponseToClient(client, "running.<br>");
        Serial.println(parsedRequest.id);
        Serial.println(parsedRequest.ml);
        runPump(parsedRequest.id, parsedRequest.ml);
        client.print("done.<br></html>");
        client.flush();
    } else {
        sendResponseToClient(client, "Invalid Request.<br></html>");
    }
}
