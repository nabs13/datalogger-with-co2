///////////////////////// For Atmega Board//////////////////////
#include<Wire.h>
#include<dht.h>
#include<LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#define DHT22_PIN 2
#define DHTTYPE    DHT22     // DHT 22 (AM2302)

int humidity_reading =0;
int temperature_reading =0;
const int chipSelect = 10;
int count = 0;

String temp_sd = " ";
String hum_sd = " ";
String pressure_sd = " ";
String co2 = "";
File dataFile;


dht DHT;
LiquidCrystal_I2C lcd(0x27,16,2);
#define BMP085_ADDRESS 0x77  // I2C address of BMP085

const unsigned char OSS = 0;  // Oversampling Setting

// Calibration values
int ac1;
int ac2;
int ac3;
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1;
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 




void setup() {
Serial.begin(9600);

Wire.begin();
lcd.init();
lcd.backlight();
lcd.setCursor(0,0);
lcd.print("Initiallizing");
lcd.setCursor(0,1);
lcd.print("System");
bmp085Calibration();
delay(2000);
lcd.clear();

 if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");


}

void loop() {
  //Assumed pin A0 is Co2 pin
  int sensorPin = analogRead(A0); 
  //float temperature = bmp085GetTemperature(bmp085ReadUT()); //MUST be called first
  float pressure = bmp085GetPressure(bmp085ReadUP());
  float atm = pressure / 101325; // "standard atmosphere"
  float altitude = calcAltitude(pressure); //Uncompensated caculation - in Meters   
//////////////////////////////////////////////////////////
   int chk = DHT.read22(DHT22_PIN);
   humidity_reading = (DHT.humidity);
   temperature_reading = (DHT.temperature);

////////////////Print on LCD screen//////////////////////
   lcd.setCursor(0,0);
   lcd.print("Humidity/per:");
   lcd.setCursor(13,0);
   lcd.print(humidity_reading);
   
   lcd.setCursor(0,1);
   lcd.print("Temp/degC:");
   lcd.setCursor(12,1);
   lcd.print(temperature_reading);
   delay(2000);
   lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("ATM Pressure: ");
  lcd.setCursor(0,1);
  lcd.print(atm, 4);
  //lcd.print("atm, 4");
  delay(2000);
  lcd.clear();
  

////////////////Send to Sd Card///////////////////////
   temp_sd += String(temperature_reading);
   hum_sd += String(humidity_reading);
   pressure_sd += String(atm);
   co2 += String(sensorPin);
   
   Serial.println("passed 1");
   //if(temperature_reading > 0)
   //{
   dataFile = SD.open("datalog.txt", FILE_WRITE);
   Serial.println("passed 2");
   if(dataFile){

   count ++;
   
   dataFile.println("Dataset: ");
   dataFile.println(count);
   
   dataFile.println("Humidity: ");
   dataFile.println(hum_sd);
   
   dataFile.println("Temperature: ");
   dataFile.println(temp_sd);
   
   dataFile.println("Pressure: ");
   dataFile.println(pressure_sd);
   
   dataFile.print("Co2: ");
   dataFile.println(co2);

   Serial.println("done");
   
 temp_sd = " ";
 hum_sd = " ";
 pressure_sd = " ";
 co2 = " ";
 
   dataFile.close();

   
  // delay(1000);
  
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  } 
       // }   
}

void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature in deg C
float bmp085GetTemperature(unsigned int ut){
  long x1, x2;

  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up){
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  long temp = p;
  return temp;
}

// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;

  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();

  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT(){
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP(){

  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = bmp085Read(0xF6);
  lsb = bmp085Read(0xF7);
  xlsb = bmp085Read(0xF8);

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  return up;
}

void writeRegister(int deviceAddress, byte address, byte val) {
  Wire.beginTransmission(deviceAddress); // start transmission to device 
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

int readRegister(int deviceAddress, byte address){

  int v;
  Wire.beginTransmission(deviceAddress);
  Wire.write(address); // register to read
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, 1); // read a byte

  while(!Wire.available()) {
    // waiting
  }

  v = Wire.read();
  return v;
}

float calcAltitude(float pressure){

  float A = pressure/101325;
  float B = 1/5.25588;
  float C = pow(A,B);
  C = 1 - C;
  C = C /0.0000225577;

  return C;
}
