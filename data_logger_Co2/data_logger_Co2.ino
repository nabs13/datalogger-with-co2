//////////////////////////////For Nano boards//////////////////////////////////////////
////////////////////////////Run this on only the Board with the nano///////////////////
#include <SPI.h>
#include <SD.h>
#include <dht.h>
dht DHT;
#define DHT22_PIN 2
const int chipSelect = 4;
int temp_reading = 0;
int hum_reading = 0;
String temp = "";
String hum = "";
String co2 = "";
int count=0; 

void setup() {
    // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void loop() {
  
  //Reading Co2 values Assumed pin A0 is Co2 pin

  int sensorPin = analogRead(A0); 
  //Serial.print("DHT22, \t");
int chk = DHT.read22(DHT22_PIN);

Serial.print("Humidity ");
hum_reading = (DHT.humidity);
Serial.println(hum_reading);
//Serial.print(",\t");
Serial.print("Temperature  ");
temp_reading = (DHT.temperature);
Serial.println(temp_reading);

Serial.print("Co2 ppm: ");
Serial.println(sensorPin);
delay(2000);

if(temp_reading>0){
  
temp += String(temp_reading);
hum += String(hum_reading);
co2 += String(sensorPin);

  // if the file is available, write to it:
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    
    count ++;
      
    dataFile.print("Data Reading number: ");
    dataFile.println(count);
   
    dataFile.print("Temperature  ");
    dataFile.println(temp);
    //dataFile.print(" ");
    dataFile.print("Humidity  ");
    dataFile.println(hum);
    
    dataFile.print("Co2: ");
    dataFile.println(co2);
    
    dataFile.close();
    
    temp = " ";
    hum = " ";
    co2 = " ";
    Serial.println("sucess");
    
    
    // print to the serial port too:
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}

}
