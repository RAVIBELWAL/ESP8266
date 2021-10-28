#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "iot"; //your ssid 
const char* password = "123456789";     // password

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String sysState="OFF";
String tankState="WAIT";
// Assign output variables to GPIO pins
int sys = 2;
const int tankwire = 0;
const int checksys;
int check;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 1000;

void setup() {

  // Initialize the output variables as outputs
  pinMode(sys, OUTPUT);
  pinMode(tankwire, INPUT);
  digitalWrite(sys, HIGH);
  check=digitalRead(tankwire);
  checksys=digitalRead(sys);

  // Connect to Wi-Fi network with SSID and password
    WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress IP=WiFi.softAPIP();
  server.begin();
}

void loop(){
  check=digitalRead(tankwire);
  checksys=digitalRead(sys);
    
    if(check==HIGH){tankState="FULL";}
    else{tankState="WAIT";}
    
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then         // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /2/ON") >= 0) {
              sysState = "ON";
              digitalWrite(sys, HIGH);
            } else if (header.indexOf("GET /2/OFF") >= 0) {
              sysState = "OFF";
              digitalWrite(sys, LOW);
            } else if (header.indexOf("GET /0/F") >= 0) {
              tankState = "FULL";
              digitalWrite(tankwire, HIGH);
            } else if (header.indexOf("GET /0/W") >= 0) {
              tankState = "WAIT";
              digitalWrite(tankwire, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv='refresh' content='2' name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Water Tank Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 0 
            client.println("<p>System State " + sysState + "</p>");
            // If the sysState is off, it displays the ON button       
            if (sysState=="OFF") {
              client.println("<p><a href=\"/2/ON\"><button class=\"button\">ON</button></a></p>");
              digitalWrite(sys,HIGH);
            } else {
              client.println("<p><a href=\"/2/OFF\"><button class=\"button button2\">OFF</button></a></p>");
            digitalWrite(sys,LOW);
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>Tank Status </p>");
            // If the tankwireState is off, it displays the ON button       
            if (tankState=="ON") {
              client.println("<p><button class=\"button\">FULL</button></a></p>");
            } else {
              client.println("<p><button class=\"button button2\">WAIT</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();

  }
}

