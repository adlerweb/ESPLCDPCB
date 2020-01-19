#include <LiquidCrystal.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char *ssid = "freifunk-myk.de";
const char *password = "";
const char *netname = "esp-lcd";

ESP8266WebServer server ( 80 );

const int rs = D0, en = D2, d4 = D3, d5 = D4, d6 = D5, d7 = D6, rw = D1, blight = D7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 LCD Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>ESP8266 LCD Demo</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"Text:<br>"
"1: <INPUT type=\"text\" name=\"text1\" value=\"\"><BR>"
"2: <INPUT type=\"text\" name=\"text2\" value=\"\"><BR>"
"Hintergrund:<br>"
"<INPUT type=\"radio\" name=\"LED\" value=\"1\">On<BR>"
"<INPUT type=\"radio\" name=\"LED\" value=\"0\">Off<BR>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>"
"</html>";

void handleRoot()
{
  if (server.hasArg("text1")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", INDEX_HTML);
  }
}

void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}

void handleSubmit()
{
  String LEDvalue;

  if (server.hasArg("LED")) {
    LEDvalue = server.arg("LED");
    if (LEDvalue == "1") {
      digitalWrite(blight, LOW);
    }
    else if (LEDvalue == "0") {
      digitalWrite(blight, HIGH);
    }
    else {
      Serial.print("Arg LED failed: ");
      Serial.println(LEDvalue);
    }
  }
  
  if (server.hasArg("text1") || server.hasArg("text2")) {
    lcd.clear();
  }

  if (server.hasArg("text1")) {
    lcd.setCursor(0, 0);
    lcd.print(server.arg("text1"));
  }
  if (server.hasArg("text2")) {
    lcd.setCursor(0, 1);
    lcd.print(server.arg("text2"));
  }
  
  server.send(200, "text/html", INDEX_HTML);
}

void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  
  //RW - always GND
  pinMode(rw, OUTPUT);
  digitalWrite(rw, LOW);
  
  //Backlight - default GND/ON
  pinMode(blight, OUTPUT);
  digitalWrite(blight, LOW);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print(ssid);
  lcd.setCursor(0, 1);
  lcd.print("Connect...");
  lcd.setCursor(0, 1);
  WiFi.mode ( WIFI_STA );
  WiFi.begin ( ssid, password );
  
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  
  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
  lcd.print(WiFi.localIP());

  if ( MDNS.begin ( netname ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  delay(1500);
}

void loop() {
  server.handleClient();
  if(Serial.available()) {
    char in = Serial.read();
    if(in == '~') {
      lcd.setCursor(0, 0);
    }else{
      lcd.print(in);
    }
  }
}
