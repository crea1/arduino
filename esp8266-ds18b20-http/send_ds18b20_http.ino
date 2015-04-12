#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
/**
make RX Arduino line is pin 2, make TX Arduino line is pin 3.
This means that you need to connect the TX line from the esp to the Arduino's pin 2
and the RX line from the esp to the Arduino's pin 3
*/
SoftwareSerial esp8266(2, 3);
#define CONNECT "AT+CWJAP=\"WIFINAMEHERE\",\"PASSWORDHERE\""
#define DEST_HOST "thingspeak.com"
#define DEST_IP "184.106.153.149"

// DS18S20 Temperature chip i/o
#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(9600);
  // Set up ds18b20
  sensors.begin();
  // Enable wifi
  esp8266.begin(9600); // your esp's baud rate might be different
  connectWifi();
  
}
void loop()
{
  sendHttpRequest();
  delay(20000);
}

void sendHttpRequest() {
  String tcpcmd = "AT+CIPSTART=\"TCP\",\"";
  tcpcmd += DEST_IP;
  tcpcmd += "\",80";
  sendATCommand(tcpcmd, 5000);
  if (esp8266.find("error")) {
    Serial.println("tcp error");
  }  
  
  // Read temperature from ds18b20 and construct url to send to Thingspeak
  sensors.requestTemperatures();
  String temperature = String(sensors.getTempCByIndex(0));
  String cmd = String("GET /update?key=THINGSPEAKIDHERE&field1=" + temperature);
  // Send length of the command we're about to send
  String cmdSend = "AT+CIPSEND=";
  cmdSend += cmd.length() + 2;
  sendATCommand(cmdSend, 5000);
  if (esp8266.find(">")) {
    esp8266.println(cmd);
    printDebug();
  } else {
    sendATCommand("AT+CIPCLOSE", 5000);
  }
}

/**
 * Send commands to esp8266 and add a delay to not send commands
 * to quickly as the esp8266 takes some time to process.
 */
void sendATCommand(String cmd, int msDelay) {
  Serial.println(cmd);
  esp8266.println(cmd);
  delay(msDelay);  
}

boolean connectWifi(){
  delay(2000);
  sendATCommand("AT+CWMODE=3", 5000);
  if (esp8266.find("no change")) {
    Serial.println("no change...");
  }
  sendATCommand("AT+RST", 2000);
  sendATCommand(CONNECT, 8000);  
  if (esp8266.find("OK")) {
    Serial.println("Connection OK");
    return true;
  } else {
    Serial.println("Connection error");
    return false;
  }
}

/**
 * Prints out response from the ESP8266
 */
void printDebug() {
if (esp8266.available()) // check if the esp is sending a message
  {
    long deadline = millis() + 5000;
    while(esp8266.available())  
    {
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      Serial.write(c);
    }
  }  
}

