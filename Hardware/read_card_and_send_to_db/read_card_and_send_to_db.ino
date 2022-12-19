/*
  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #                                                               #
  #                 Installation :                                      #
  # NodeMCU ESP8266/ESP12E    RFID MFRC522 / RC522                      #
  #         D2       <---------->   SDA/SS                              #
  #         D5       <---------->   SCK                                 #
  #         D7       <---------->   MOSI                                #
  #         D6       <---------->   MISO                                #
  #         GND      <---------->   GND                                 #
  #         D1       <---------->   RST                                 #
  #         3V/3V3   <---------->   3.3V                                #
  #         D0       <---------->   LED and buzzer pin                  #
  # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
*/

//----Include the NodeMCU ESP8266 Library---//
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

//Include the library for the RFID Reader
#include <SPI.h>
#include <MFRC522.h>

//define the pin numbers
#define SS_PIN 4  //--> SDA / SS is connected to pinout D2
#define RST_PIN 5  //--> RST is connected to pinout D1

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router
#define Buzzer 16 // D0 pin for the buzzer

MFRC522 mfrc522(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.

int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;

//-----SSID and Password of the access point you want to create from the system-------//
const char* ssid = "For Payment";
const char* password = "0987654321";

//set the endpoint that data will be dropped
const String apikey = "somade_daniel";
const String servername = "http://192.168.4.2/nfc_payment/backend/recieveReadings/receive.php";

//add api key to the name
const String serverApi = servername + "?apikey=" + String(apikey);

ESP8266WebServer server(80);  //--> Server on port 80

void setup() {
  Serial.begin(115200); //--> Initialize serial communications with the PC
  
  SPI.begin();      //--> Init SPI bus
  
  mfrc522.PCD_Init(); //--> Init MFRC522 card

  delay(500);
  
  pinMode(ON_Board_LED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board
  digitalWrite(Buzzer, LOW);

//create the access point
  WiFi.softAP(ssid, password);
  Serial.print("Access Point: ");
  Serial.print(ssid); Serial.println(" ...");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP()); 
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("");
  Serial.println("Please tag a card or keychain to see the UID !");
  Serial.println("");
}

void loop() {
  readsuccess = getid();

  if (readsuccess) {
    digitalWrite(ON_Board_LED, LOW);
    digitalWrite(Buzzer, HIGH);

    String UIDresultSend, postData;
    UIDresultSend = StrUID; //get the id of the card
    Serial.println(UIDresultSend); //display the id of the card in the serial monitor

    //concantenate readings with endpoint to send data
    String request = serverApi + "&card_number=" + String(UIDresultSend);
    Serial.print("Request: "); Serial.print(request);
    Serial.println("");

    //Declare object of class HTTPClient
    WiFiClient client;
    HTTPClient http;    
    http.begin(client,request.c_str());  //Specify request destination

    int httpResponseCode = http.GET();   //Send the request

    if(httpResponseCode > 1){
      //diplay response of request from server 
      String payload = http.getString();
      Serial.println(payload);
    }else{
      //display error code is request is not sent
      Serial.print("Error code: "); Serial.print(httpResponseCode);
    }

    http.end();  //Close connection

    delay(500);
    digitalWrite(ON_Board_LED, HIGH);
    digitalWrite(Buzzer, LOW);
  }
}

//----------------Procedure for reading and obtaining a UID from a card or keychain----------//
int getid() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }

  Serial.print("THE UID OF THE SCANNED CARD IS : ");

  for (int i = 0; i < 4; i++) {
    readcard[i] = mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str;
  }
  mfrc522.PICC_HaltA();
  return 1;
}

//----Procedure to change the result of reading an array UID into a string----------//
void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}