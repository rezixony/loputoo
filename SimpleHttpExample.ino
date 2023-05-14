#include <SPI.h>
#include <EthernetENC.h>

#include <DHT.h>
#define DHTPIN 5 // what pin DHT11 is connected to
#define photoPin 1 // what pin photoresistor is connected to
#define MQ2pin 0 // what pin MQ2  is connected to

#define DHTTYPE DHT11  // DHT 11  (AM2302)
DHT dht(DHTPIN, DHTTYPE);


// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //should be unique on the local network

// Set the static IP address to use if the DHCP fails to assign
#define MYIPADDR 192, 168, 97, 18
#define MYIPMASK 255, 255, 248, 0
#define MYDNS 192, 168, 100, 100
#define MYGW 192, 168, 101, 1

EthernetClient client;

int HTTP_PORT = 80; //port for HTTP
String HTTP_METHOD = "POST"; // method we're using to send data over HTTP
//String PATH_NAME = "/api/?key=507acc57-8c25-11ed-acdf-0892042f3260&loc=K789&tem=56&hum=56";
String queryString = "http://iot.tbt.ee/api/?key=507acc57-8c25-11ed-acdf-0892042f3260";

 //http://iot.cola.ee/api/?key

void setup() {
  Serial.begin(9600);
  dht.begin();

  // initialize the Ethernet shield using DHCP:
  if (Ethernet.begin(mac)) {  // Dynamic IP setup
    Serial.println("DHCP OK!");
  } else {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1);  // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }

    IPAddress ip(MYIPADDR);
    IPAddress dns(MYDNS);
    IPAddress gw(MYGW);
    IPAddress sn(MYIPMASK);
    Ethernet.begin(mac, ip, dns, gw, sn);
    Serial.println("STATIC OK!");
  }
  delay(5000);

  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());
}

void loop() {
  float temp = dht.readTemperature(); //variable to store temperature value
  float humid = dht.readHumidity(); //variable to store humidity value 
  float hic = dht.computeHeatIndex(temp, humid, false); //calculation of heat index
  float gasValue = analogRead(MQ2pin);  //variable to store gas sensor value
  int lightValue = analogRead(photoPin); //variable to store photoresistor value

  
  String location = "K62-1"; //where the hardware is located

  // connect to web server on port 80:
  if (client.connect("iot.tbt.ee", HTTP_PORT)) {

    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println(HTTP_METHOD + " " + "/api/?key=507acc57-8c25-11ed-acdf-0892042f3260&loc=" + location + "&tem=" + temp + "&hum=" + humid + "&hic=" + hic + "&gas=" + gasValue + "&light=" + lightValue + " HTTP/1.1");
    client.println("Host: " + String("iot.tbt.ee"));
    client.println("Connection: close");
    client.println();  // end HTTP header

    // send HTTP body
    client.println(queryString);

    while (client.connected()) {
      if (client.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {  // if not connected:
    Serial.println("connection failed");
  }
  delay(900000); //wait for 15 minutes for the next reading
}