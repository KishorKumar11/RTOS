// Load Wi-Fi library
#include <WiFi.h>

#define RXD2 16
#define TXD2 17

// Replace with your network credentials
const char* ssid = "Justin";
const char* password = "aloysius";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String response, ip_address;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
int wait10 = 10000; // time to reconnect when connection is lost.


// This is your Static IP
IPAddress local_IP(192, 168, 253, 63); 
// Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4); 


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Configure Static IP
  if (false) {
    if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
      Serial.println("Static IP failed to configure");
    }
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  ip_address = WiFi.localIP().toString();
  Serial.println(ip_address);
  server.begin();
}

void loop() {

// If disconnected, try to reconnect every 10 seconds.
  if ((WiFi.status() != WL_CONNECTED) && (millis() > wait10)) {
    Serial.println("Trying to reconnect WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    wait10 = millis() + 10000;
  } 
  // Check if a client has connected..
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
   
  Serial.print("New client: ");
  Serial.println(client.remoteIP());

  /////////////////////////////////////////////////////
  // Read the information sent by the client.
  String req = client.readStringUntil('\r');
  Serial.println(req);

  // Make the client's request.
  if(req.indexOf("stop") != -1)
  {
    response = "Stop command received";
    Serial2.write(0x10);
  }
  if(req.indexOf("status") != -1)
  {
    response = "WiFi Connected: " + ip_address;
    Serial2.write(0x20);
  }
  if(req.indexOf("forwardStraight") != -1)
  {
    response = "Forward command received";
    Serial2.write(0x11);
  }
  if(req.indexOf("forwardLeft") != -1)
  {
    response = "Forward left command received";
    Serial2.write(0x12);
  }
  if(req.indexOf("forwardRight") != -1)
  {
    response = "Forward right command received";
    Serial2.write(0x13);
  }
  if(req.indexOf("reverseStraight") != -1)
  {
    response = "Reverse command received";
    Serial2.write(0x15);
  }  
  if(req.indexOf("reverseLeft") != -1)
  {
    response = "Reverse left command received";
    Serial2.write(0x16);
  }
  if(req.indexOf("reverseRight") != -1)
  {
    response = "Reverse right command received";
    Serial2.write(0x17);
  }
  if(req.indexOf("leftSpin") != -1)
  {
    response = "Left spin command received";
    Serial2.write(0x18);
  }
  if(req.indexOf("rightSpin") != -1)
  {
    response = "Right spin command received";
    Serial2.write(0x19);
  }
  if(req.indexOf("increaseSpeed") != -1)
  {
    response = "Increase speed command received";
    Serial2.write(0x1a);
  }
  if(req.indexOf("decreaseSpeed") != -1)
  {
    response = "Decrease speed command received";
    Serial2.write(0x1b);
  }
  if(req.indexOf("finishLevel") != -1)
  {
    response = "Finish Level command received";
    Serial2.write(0x30);
  }
  if(req.indexOf("reset") != -1)
  {
    response = "Reset command received";
    Serial2.write(0x40);
  }
  if(req.indexOf("setAuto") != -1)
  {
    response = "Set Auto command received";
    Serial2.write(0x50);
  }
  if(req.indexOf("setManual") != -1)
  {
    response = "Set Manual command received";
    Serial2.write(0x60);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println(response); //  Return status.

  client.flush();
  client.stop();
  Serial.println("Client disconnected.");
}
