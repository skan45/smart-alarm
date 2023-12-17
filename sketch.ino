#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "ESP32Servo.h"
Servo s;
int p=90;
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);
int Wifi_ReConnect=0;
int Mqtt_ReConnect=0;
const byte ROWS = 4; // Number of rows in the keypad
const byte COLS = 4; // Number of columns in the keypad
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";
const char* mqttServer = "broker.hivemq.com"; // Replace with your MQTT broker address
const int mqttPort = 1883; // MQTT broker port
const char* mqttClient= "USER001";
WiFiClient espClient;
PubSubClient client(espClient);
const char* intruderTopic = "/home/police";
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {23, 2, 0, 19}; // Connect these to the row pinouts of the keypad
byte colPins[COLS] = {18, 5, 17, 16}; // Connect these to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const String secretCode = "1234"; // Replace this with your secret 4-digit code

String enteredCode = "";
int wrongAttempts = 0;
bool keypadActive = true;
unsigned long keypadLockedUntil = 0;
void connectToWiFi() {
   LCD.init();
  LCD.backlight();
  LCD.setCursor(0,0);
  LCD.print("Connect to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LCD.print(".");
  }
   LCD.println("");
   LCD.clear();
   LCD.setCursor(0,0);
   LCD.print("Wi-Fi connected");
   delay(1000);
}
void setup() {
  Serial.begin(9600);
  connectToWiFi();
  connectmqtttopic();
  client.setCallback(callback);
  pinMode(4,OUTPUT);
  pinMode(27,INPUT);
  pinMode(15, OUTPUT);
  noTone(4);
  s.attach(25);
  s.write(p);
  Wire.begin(21,22);
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("enter code:");
}
void connectmqtttopic(){

  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(mqttClient )) {//in this case no user, no password are needed
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
   if (client.subscribe("/home/owner")) {
    Serial.print ("Suscribed to Topic: ");Serial.println("/home/owner");
    client.publish("/home/owner","connected to the topic ");
  }  
  client.publish("/home/police","first contact");
  }
  void reconnect(){
  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
      {
    Serial.println("WAITING WIFI CONNECTION .....");
    delay(1000);        
    }
      connectmqtttopic();
  }

void loop() {
   if (WiFi.status() != WL_CONNECTED) {
    Wifi_ReConnect++;
    Serial.println("WIFI CONNECTION LOST ... WAITING");
    reconnect();
      }
      
   if(digitalRead(27)==HIGH){
    //envoi MQTT
    client.publish("/home/owner","something approaching !!");
  }
  if (keypadActive) {
    char key = keypad.getKey();

    if (key != NO_KEY && enteredCode.length()< 4) {
      enteredCode += key;
      LCD.setCursor(0, 10);
      LCD.print(enteredCode);
    }
    if (enteredCode.length() == 4) {
      if (enteredCode == secretCode) {
        LCD.clear();
        LCD.setCursor(0, 0);
        LCD.print("Code accepted!");
        s.write(0);
        delay(20000);
        s.write(p);
        LCD.clear();
        LCD.setCursor(0, 0);
        LCD.print("enter code:");
        wrongAttempts = 0;
        // Add your code to perform actions when the correct code is entered
      } else {
       LCD.clear();
       LCD.setCursor(0, 0);
       LCD.print("Incorrect code!");
       wrongAttempts++;
       if (wrongAttempts >=3) {
          keypadActive = false;
          client.publish("/home/police","intruder alert need help immediately !!!!!");
          client.publish("/home/police","intruder alert need help immediately !!!!!");
          client.publish("/home/police","intruder alert need help immediately !!!!!");
          wrongAttempts = 0; 
          tone(4, 3000); 
           digitalWrite(15, HIGH);
          LCD.clear();
          LCD.setCursor(0, 0);
          LCD.print("Keypad locked ");
           delay(20000); 
           noTone(4); 
           digitalWrite(15, LOW);
          keypadLockedUntil = millis() + 10000;
                  }
       delay(3000);
       LCD.clear();
       LCD.setCursor(0, 0);
       LCD.print("enter code:");
        
      }
      enteredCode = ""; 
    }
  } else {
    if (millis() >= keypadLockedUntil) {
      keypadActive = true;
     LCD.clear();
          LCD.setCursor(0, 0);
          LCD.print("Keypad unlocked");
          delay(5000);
          LCD.clear();
          LCD.setCursor(0, 0);
          LCD.print("enter code:");

    }
  }
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.println("Message arrived in topic: " + String(topic));
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();

  // String receivedMessage = "";
  // for (int i = 0; i < length; i++) {
  //   receivedMessage += (char)message[i];
  // }
  // Serial.print("Received Message: ");
  // Serial.println(receivedMessage); // Print the received message

  // if (receivedMessage.equals("STOP")) {
  //   noTone(4); 
  //   digitalWrite(15, LOW); 
  //   Serial.println("Alarm stopped!");
  // } else {
  //   Serial.println("Message does not match 'STOP'");
  // }
}
