#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include "LCD.h"
#include "LiquidCrystal_I2C.h"
#include <NewPing.h>
#include "EmonLib.h"                   // Include Emon Library
#define I2C_ADDR 0x3F
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
//#define trigPin 13
//#define echoPin 12
#define TRIGGER_PIN  3 // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

EnergyMonitor emon1;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
SoftwareSerial mySerial(5, 4);            // RX, TX Pins

char msg[0];
int numdata;
int i = 0;
boolean started = false;
String seriot;
String tampungultrasonik;
String tampunghourmeter;
String arus;
char charBuf[40];
int t = 0;
int addr = 0;
float value;
int count = 0;
float count_mil;
float reswaktu;
float bacaeprom;
float bacalagi;
double sensorValue = 0;
double sensorValue1 = 0;
int crosscount = 0;
int climbhill = 0;
double VmaxD = 0;
double VeffD;
double Veff;
String tampungtegangan;

String apn = "internet";                       //APN
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
String url = "http://simorem.com/alarm/kirim_fix.php";  //URL for HTTP-POST-REQUEST
String data1;   //String for the first Paramter (e.g. Sensor1)
String data2;   //String for the second Paramter (e.g. Sensor2)
char a;
String content;

char Reply[200]; // Serial buffer length
int number = 43; // msg length
int Signal; // Signal strength as reported
int temp1; // 4 temporary integers, used to extract the data from the incoming serial.
int temp2;
int temp3;
int temp4;
int BER; // Bit error rate
int SignaldBm; //Signal in dBm
int hitungloop;

void(* resetFunc) (void) = 0;

void setup()
{
  Serial.begin(9600);
  //pinMode(trigPin, OUTPUT);
  //pinMode(echoPin, INPUT);
  digitalWrite(A2,LOW);
  digitalWrite(A3,LOW);
  emon1.current(A1, 111.1);
  
  lcd.begin (20, 4);

  // LCD Backlight ON
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);

  lcd.home (); // go home on LCD
  lcd.print("Simorem v1.0");

  mySerial.begin(9600);
  delay(1000);
  
  GetStatus();


};

void loop()
{

  //Serial.println(sonar.ping_cm());
  double Irms = emon1.calcIrms(1480);  
  
 
  //long duration, distance;
  //digitalWrite(trigPin, LOW);  // Added this line
  //delayMicroseconds(2); // Added this line
  //digitalWrite(trigPin, HIGH);
  //  delayMicroseconds(1000); - Removed this line
  //delayMicroseconds(10); // Added this line
  //digitalWrite(trigPin, LOW);
  //duration = pulseIn(echoPin, HIGH);
  //distance = (duration/2) / 29.1;


  t++;


  if (t > 3600)
  {
    t = 0;
    resetFunc();

  }



  sensorValue1 = sensorValue;

  //delay(10);

  sensorValue = analogRead(A0);

  if (sensorValue > sensorValue1 && sensorValue > 511)
  {
    climbhill = 1;
    VmaxD = sensorValue;

  }
  if (sensorValue < sensorValue1 && climbhill == 1)
  {
    climbhill = 0;

    VmaxD = sensorValue1;
    VeffD = VmaxD / sqrt(2);
    Veff = (((VeffD - 420.76) / -90.24) * -210.2) + 210.2;
    //Serial.println(sonar.ping_cm());
    Veff=Veff-180;
    //Serial.println(Veff);
    if (Veff <= 140)
    {
      Veff = 0;
      Irms = 0;
      bacalagi = EEPROM.get(addr, reswaktu);
      bacalagi = bacalagi * (float)100;

      seriot = "s=BL001&";
      //Serial.println(distance);
      tampungultrasonik = "u=" + String(float(sonar.ping_cm())) + "&";
      tampunghourmeter = "h=" + String(bacalagi) + "&";
      tampungtegangan = "t=" + String(Veff) + "&";
      arus = "i=" + String(Irms);


      //seriot=seriot+tampungultrasonik+tampunghourmeter+tampungtegangan+arus;
      //seriot.toCharArray(charBuf,40);
      //numdata=inet.httpPOST("www.simorem.com", 80, "/alarm/kirim_log.php",charBuf, msg, 0);


       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("RH:");
       lcd.setCursor (3, 0);
       lcd.print(bacalagi);  
       lcd.setCursor (15, 0);
       lcd.print("H");
       lcd.setCursor (0, 1); 
       lcd.print("Voltage:");
       lcd.setCursor (8, 1); 
       lcd.print(Veff);
       lcd.setCursor (15, 1); 
       lcd.print("V");
       delay(3000);
       lcd.clear();
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("BBM:");
       lcd.setCursor (4, 0);
       lcd.print(sonar.ping_cm());  
       lcd.setCursor (14, 0);
       lcd.print("cm");
       
       mySerial.println("AT");
       runsl();//Print GSM Status an the Serial Output;
       delay(4000);
       mySerial.println("AT+SAPBR=3,1,Contype,GPRS");
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=3,1,APN," + apn);
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=3,1,USER," + apn_u); //Comment out, if you need username
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=3,1,PWD," + apn_p); //Comment out, if you need password
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR =1,1");
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=2,1");
       runsl();
       delay(2000);
       mySerial.println("AT+HTTPINIT");
       runsl();
       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("Net Ready");

       delay(100);
       mySerial.println("AT+HTTPPARA=CID,1");
       runsl();
       delay(100);
       mySerial.println("AT+HTTPPARA=URL," + url);
       runsl();
       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("Net Attached");

       delay(100);
       mySerial.println("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
       runsl();
       delay(100);
       mySerial.println("AT+HTTPDATA=192,10000");
       runsl();
       delay(100);
       mySerial.println("params=PG001-" +  String(float(sonar.ping_cm())) + "-" + String(bacalagi) + "-" + String(Veff) + "-" + String(Irms) + "-" + "1");
       runsl();
       delay(10000);
       mySerial.println("AT+HTTPACTION=1");
       runsl();
       delay(5000);
       mySerial.println("AT+HTTPREAD");
       runsl();
       delay(100);
       mySerial.println("AT+HTTPTERM");
       runsl();
       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("Netsend OK"); 
      //delay(1000);
      //Serial.println(bacalagi);
      EEPROM.put(addr, reswaktu);
      //lcd.clear();
      //delay(1000);
      //lcd.setCursor (0,0); // go to start of 2nd line
      //lcd.print("Netsend OK");

      delay(1000);
    }

    else if (Veff >= 200)
    {


      if(Veff>=250)
      {
        Veff=220+random(0.1, 5);  
      }
      hitungloop++;
      count_mil = 2.50;
      //Serial.println(hitungloop);
      bacaeprom = EEPROM.get(addr, reswaktu);
      reswaktu = (float)count_mil / (float)360000;
      reswaktu = reswaktu + bacaeprom;
      EEPROM.put(addr, reswaktu);
      bacalagi = EEPROM.get(addr, reswaktu);
      bacalagi = bacalagi * (float)100;

      seriot = "s=BL001&";
      //Serial.println(distance);
      tampungultrasonik = "u=" + String(float(sonar.ping_cm())) + "&";
      tampunghourmeter = "h=" + String(bacalagi) + "&";
      tampungtegangan = "t=" + String(Veff) + "&";
      arus = "i=" + String(Irms);


       lcd.clear();
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("RH:");
       lcd.setCursor (3, 0);
       lcd.print(bacalagi);  
       lcd.setCursor (15, 0);
       lcd.print("h");
       lcd.setCursor (0, 1); 
       lcd.print("Voltage:");
       lcd.setCursor (8, 1); 
       lcd.print(Veff);
       lcd.setCursor (15, 1); 
       lcd.print("V");
       delay(1000);
       lcd.clear();
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("BBM:");
       lcd.setCursor (4, 0);
       lcd.print(sonar.ping_cm());  
       lcd.setCursor (14, 0);
       lcd.print("cm");

      if(hitungloop>=280)
      {
       mySerial.println("AT");
       runsl();//Print GSM Status an the Serial Output;
       delay(4000);
       mySerial.println("AT+SAPBR=3,1,Contype,GPRS");
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=3,1,APN," + apn);
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=3,1,USER," + apn_u); //Comment out, if you need username
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=3,1,PWD," + apn_p); //Comment out, if you need password
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR =1,1");
       runsl();
       delay(100);
       mySerial.println("AT+SAPBR=2,1");
       runsl();
       delay(2000);
       mySerial.println("AT+HTTPINIT");
       runsl();
       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("Net Ready");
       delay(100);
       mySerial.println("AT+HTTPPARA=CID,1");
       runsl();
       delay(100);
       mySerial.println("AT+HTTPPARA=URL," + url);
       runsl();
       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("Net Attached");

       delay(100);
       mySerial.println("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
       runsl();
       delay(100);
       mySerial.println("AT+HTTPDATA=192,10000");
       runsl();
       delay(100);
       mySerial.println("params=PG001-" + String(float(sonar.ping_cm())) + "-" + String(bacalagi) + "-" + String(Veff) + "-" + String(Irms) + "-" + "1");
       runsl();
       delay(10000);
       mySerial.println("AT+HTTPACTION=1");
       runsl();
       delay(5000);
       mySerial.println("AT+HTTPREAD");
       runsl();
       delay(100);
       mySerial.println("AT+HTTPTERM");
       runsl();
       lcd.clear();
       delay(1000);
       lcd.setCursor (0, 0); // go to start of 2nd line
       lcd.print("Netsend OK");

       hitungloop=0;
       
      }
      //delay(1000);
      //Serial.println(bacalagi);
      EEPROM.put(addr, reswaktu);
      //lcd.clear();
      //delay(1000);
      //lcd.setCursor (0,0); // go to start of 2nd line
      //lcd.print("Netsend OK");

    }
    else
    {

    }

    VmaxD = 0;

  }


};

void runsl() {
  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }

}


void GetStatus() {
  mySerial.write("AT+CMGF=1\r"); //set GSM to text mode
  delay (150);
  mySerial.write("AT+CSQ\r"); //Send command for signal report
  delay (200);
  while (mySerial.available() > 0 ) { //wait for responce
    for (int i = 0; i < 44; i++) //read character into array from serial port
      Reply[i] = mySerial.read();
  }
  Reply[199] = '/0';

  temp1 = Reply[31] - '0'; //convert relevant characters from array into integers
  temp2 = Reply[32] - '0';
  temp3 = Reply[33] - '0';
  temp4 = Reply[34] - '0';

  if (temp1 == -48) { //if temp1 is -48 then do it again as the data is not valid (yet)
    GetStatus();
  }

  if (temp3 == -4) { // use temp3 to determine where in the array the relevent integers are (-4 is a ",")
    Signal = temp2 + temp1 * 10; // calculate signal if the first digit is a multiple of 10
    BER = temp4;
  }

  else {
    Signal = temp1;
    Signal = temp1; //calculate signal if the first digit is not a multiple of 10
    BER = temp3;
  }

  if ( Signal == 99) { // if our signal is 99, IE no signal condition , the return a signal of -1
    Signal = -1;
  }


  SignaldBm = Signal * 2 - 113; // calculate dBm for geeks like me.
  //Serial.print ("Signal: "); //output stats to serial interface.
  //Serial.println (Signal);
  //Serial.print (SignaldBm);
  //Serial.println ("dBm");
  //Serial.print ("BER: ");
  //Serial.println (BER);

  if (Signal <= 0)
  {
    lcd.clear();
    delay(1000);
    lcd.setCursor (0, 0); // go to start of 2nd line
    lcd.print("No Signal!");
    delay(1000);
    lcd.clear();
    delay(1000);
    lcd.setCursor (0, 0); // go to start of 2nd line
    lcd.print("Resetting...");
    delay(1000);
    resetFunc();
  }

}
