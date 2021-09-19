#include <SPI.h>
#include <MFRC522.h>
#include <TinyGPS++.h> 
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(4, 5);

float latitude , longitude;
String lat_str , lng_str;

const char *ssid = "RAVI PRASAD";  //ENTER YOUR WIFI SETTINGS
const char *password = "27406041";

const char *host = "til7kcps8l.execute-api.ap-south-1.amazonaws.com";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "1B 60 AB 6E AB 98 EC 21 C2 B7 C1 A5 AA 76 F2 9A 56 8A 4C 15";
//=======================================================================
//                    Power on setup
//=======================================================================
 
#define SS_PIN 2
#define RST_PIN 0
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 
void setup() 
{
  
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin(); 
  ss.begin(9600);// Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println();
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

}
void loop() 
{
  
  
  while (ss.available() > 0)
    if (gps.encode(ss.read())) //read gps data
    {
      if (gps.location.isValid()) //check whether gps location is valid
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6); // latitude location is stored in a string
        longitude = gps.location.lng();
        lng_str = String(longitude , 6); //longitude location is stored in a string
        
      }
}
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "C7 BA B7 4D") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println(lat_str);
    Serial.println(lng_str);
    Serial.println();
    WiFiClientSecure httpsClient;  //Declare object of class WiFiClient
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
     Serial.print("HTTPS Connecting");
  int r = 0; //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (r < 30)) {
    delay(100);
    Serial.print(".");
    r++;
  }
  if (r == 30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
    String getData, Link;

  //POST Data
  Link = "/";

  Serial.print("requesting URL: ");
  Serial.println(host);
  //   String data = "{\"device_id\":\"yomama\",\"lat\":\"" +(String)lat+"\",\"long\":\"" +(String)lon+"\"}";
  String data = "{\"device_id\":\"yomama\",\"lat\":\"" + String(latitude) + "\",\"long\":\"" + String(longitude) + "\"}";
  String req = String("POST ") + Link + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Content-Type: application/json" + "\r\n" + "Content-Length: " + (String)data.length() + "\r\n\r\n" + data + "\r\n" + "Connection: close\r\n\r\n";
  Serial.println(req);
  httpsClient.print(req);

  Serial.println("request sent");

  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while (httpsClient.available()) {
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
    break;
  }
  Serial.println("==========");
  Serial.println("closing connection");

  delay(10000);  //POST Data at every 2 seconds
  }
 
 else   {
    Serial.println(" Access denied");
    delay(1000);
  }
} 
