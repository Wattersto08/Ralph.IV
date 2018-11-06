 

#include <LiquidCrystal.h>
#include <Servo.h>

#include "DHT.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------- GPIO -----------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DHTPIN 8     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11


  DHT dht(DHTPIN, DHTTYPE);

  float h;
  float t;
  float f;
  
// Mode Select Knob
  int Kpin = A0;  
  int Kval = 0;

// RC Position mathmatics
  int midpoint = 1450;
  int midbuffer = 15;
  int pwmRange = 375;
  int Yp;
  int Xp;
  int HYp;
  float  Turnmod = 1.5;
  int Lspd;
  int Rspd;
  int var;

//PING Sensors
  const int trigPinA = 11;
  const int echoPinA = 12;
  const int trigPinB = 34;
  const int echoPinB = 35;
  long durationA;
  int distanceA;
  long durationB;
  int distanceB;

  int var1=0;
  int thresh = 15;
  int Sensitivity = 3;

//RGB LED IO
  int LEDG = 46;
  int LEDR = 45;
  int LEDB = 47;

//Interupt Loop Values
  volatile unsigned long pwmX;
  volatile boolean done1;
  unsigned long start1;

  volatile unsigned long pwmY;
  volatile boolean done2;
  unsigned long start2;
  unsigned long currentMillis;

  long previousMillis = 0;    // set up timers
  long interval = 40;        // time constant for timers

// L298d Motor Driver Config
//Right motor
  int enA = 52;
  int in1 = 51;
  int in2 = 49;
//Left motor
  int enB = 48;
  int in3 = 53;
  int in4 = 50;

// Autonomy Values

  int motspeed = 200;

//LCD config
  const int rs = 26, en = 27, d4 = 22, d5 = 23, d6 = 24, d7 = 25;
  LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Servo Config
  Servo myservo;


////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------- Setup ----------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  
  pinMode(2, INPUT);
  attachInterrupt(0, timeitX, CHANGE);
  
  pinMode(3, INPUT);
  attachInterrupt(1, timeitY, CHANGE);
  
  lcd.begin(16, 2);
  lcd.print(" Ralph - mk.IV");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print("  By Tom Watters");
  
  pinMode(trigPinA, OUTPUT); 
  pinMode(echoPinA, INPUT); 
  pinMode(trigPinB, OUTPUT); 
  pinMode(echoPinB, INPUT); 

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  myservo.attach(6);
  dht.begin();

  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDB, OUTPUT);

  Serial.begin(115200);

  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  delay(100);
  
}


void timeitX() {
    if (digitalRead(2) == HIGH) {
      start1 = micros();
    }
    else {
      pwmX = micros() - start1;
      done1 = true;
    }
  }

void timeitY() {
    if (digitalRead(3) == HIGH) {
      start2 = micros();
    }
    else {
      pwmY = micros() - start2;
      done2 = true;
    }
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------- Main Loop ------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() { 


  Kval = analogRead(Kpin);
  Kval = constrain(Kval, 0, 1000);
  
    if(Kval<5){
      var = 0;
    }
    else if(Kval>5 and Kval<200){
      var=1;
    }
    else if(Kval>200 and Kval<400){
      var=2;
    }
    else if(Kval>400 and Kval<600){
      var=3;
    }
    else if(Kval>600 and Kval<800){
      var=4;
    }
    else if(Kval>800 and Kval<999){
      var=5;
    }
    else if(Kval>999){
      var=6;
    }


  switch (var) {
    case 1:
          while(Kval>5 and Kval<200){
              lcd.setCursor(0, 0);
              lcd.print("1: RC Control  ");
              RCControl();
              Kval = analogRead(Kpin);
          }
    break;
       
    case 2:
          LEDoff();
          STOP();
          Serial.println("Mode 2");
          lcd.setCursor(0, 0);
          lcd.print("2: Autonomous   ");
          lcd.setCursor(0,1);
          lcd.print("                 ");
          delay(50); 
    break;

    case 3:
          while(Kval>400 and Kval<600){
              STOP();
              lcd.setCursor(0, 0);
              lcd.print("3: Ping Readout: ");
              Objdetect();
              lcd.setCursor(2,1);
              lcd.print("L:");
              lcd.print(distanceA);
              lcd.print("  ");
              lcd.setCursor(8,1);
              lcd.print(" R:");
              lcd.print(distanceB);
              lcd.print("   ");
              Kval = analogRead(Kpin);
          }
    break;
      
    case 4:
          STOP();
          LEDoff();
          TempCheck();
          Serial.println("Mode 4");
          lcd.setCursor(0, 0);
          lcd.print("4:    DHT22      ");
         
          lcd.setCursor(1,1);
          lcd.print("T:");
          lcd.print(t);
          lcd.setCursor(5,1);
          lcd.print("'C - H:");
          lcd.print(h);
          lcd.setCursor(14,1);
         
          lcd.print("% ");
          delay(50);
    break;

    case 5:
       while((Kval>800 and Kval<999)){
             
          STOP();
             
          lcd.setCursor(0, 0);
          lcd.print("5: Blank Mode   ");
          lcd.setCursor(0,1);
          lcd.print("               ");
             
          delay(20);

          Kval = analogRead(Kpin);
       
    
         }
             
    break;
    
    case 6:
          STOP();
          Serial.println("Light Show");
          lcd.setCursor(0, 0);
          lcd.print("6: Light Show       ");
          lcd.setCursor(0,1);
          lcd.print("                "); 
          STOP();
          Lightshow();
    break; 

      
    default:
          STOP();
          LEDoff();
          Serial.println("Stop");
          lcd.setCursor(0, 0);
          lcd.print("  Ralph mk.IV      ");
          lcd.setCursor(0,1);
          lcd.print(" -mode select-    "); 
          delay(100);
    break;
    
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------- SubRoutines ------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////////

void TempCheck(){
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
}

void FWD(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);  
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);   
  }

void REV(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);  
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);   
  }

void STOP(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);  
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);  
  }


void PingL(){
  // Clears the trigPinA
  digitalWrite(trigPinA, LOW);
  delayMicroseconds(2);
  // Sets the trigPinA on HIGH state for 10 micro seconds
  digitalWrite(trigPinA, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinA, LOW);
  // Reads the echoPinA, returns the sound wave travel time in microseconds


  durationA = pulseIn(echoPinA, HIGH);
  // Calculating the distanceA
  distanceA= durationA*0.034/2;
  // Prints the distanceAA on the Serial Monitor
  distanceA = constrain(distanceA,0,100);
}

void PingR(){
  digitalWrite(trigPinB, LOW);
  delayMicroseconds(2);
  // Sets the trigPinA on HIGH state for 10 micro seconds
  digitalWrite(trigPinB, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinB, LOW);
  // Reads the echoPinA, returns the sound wave travel time in microseconds


  durationB = pulseIn(echoPinB, HIGH);
  // Calculating the distanceA
  distanceB= durationB*0.034/2;
  // Prints the distanceAA on the Serial Mon
  distanceB = constrain(distanceB,0,100);
}

void LEDRED(){
      digitalWrite(LEDG,LOW);
      digitalWrite(LEDB,LOW);
      digitalWrite(LEDR,HIGH);
  }
void LEDGREEN(){
      digitalWrite(LEDG,HIGH);
      digitalWrite(LEDB,LOW);
      digitalWrite(LEDR,LOW);
  }
void LEDBLUE(){
      digitalWrite(LEDG,LOW);
      digitalWrite(LEDB,HIGH);
      digitalWrite(LEDR,LOW);
  }
void LEDoff(){
      digitalWrite(LEDG,LOW);
      digitalWrite(LEDB,LOW);
      digitalWrite(LEDR,LOW);   
  }

void Lightshow(){
      digitalWrite(LEDG,LOW);
      digitalWrite(LEDB,HIGH);
      digitalWrite(LEDR,LOW);
      delay(100);
      digitalWrite(LEDG,HIGH);
      digitalWrite(LEDB,LOW);
      digitalWrite(LEDR,LOW);
      delay(100);
      digitalWrite(LEDG,LOW);
      digitalWrite(LEDB,LOW);
      digitalWrite(LEDR,HIGH);
      delay(100);
      
      
  }

 void RCControl(){

  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {  //start1 timed event
      
      previousMillis = currentMillis;
      
      Serial.print (pwmX);
      Serial.print (" ");
      Serial.print(pwmY);
      Serial.print (" ");
 
 if(pwmY>(midpoint+midbuffer)){
      LEDGREEN();
      Yp = pwmY-midpoint;
      Yp = map(Yp, 0, pwmRange, 0, 255);
      Yp = constrain(Yp, 0, 255);
      Serial.print("FWD: ");
      Serial.println(Yp);
      
      lcd.setCursor(0, 1);
      lcd.print("FWD: ");  
      FWD();
      
      Lspd = Yp;
      Rspd = Yp;
     
 
  }
 else if(pwmY<(midpoint-midbuffer)){
      LEDRED();
      Yp = midpoint-pwmY;
      Yp = map(Yp, 0, pwmRange, 0, 255);
      Yp = constrain(Yp, 0, 255);

      Serial.print("REV: ");
      Serial.println(Yp);

      lcd.setCursor(0, 1);
      lcd.print("REV: ");
      REV();
      Lspd = Yp;
      Rspd = Yp;
 
  }
 else if(pwmY<(midpoint+midbuffer)and pwmY>(midpoint-midbuffer)){
      LEDBLUE();
      Serial.println(" ");
      lcd.setCursor(0, 1);
      lcd.print("Stop");
      STOP();
      Lspd = 000;
      Rspd = 000;
     
 }

if(pwmX>(midpoint+midbuffer)){
      Xp = pwmX- midpoint;            
      Xp = map(Xp, 0, pwmRange, 0, 255);
      Xp = constrain(Xp, 0, 255);
      Xp = Xp/Turnmod;

      Rspd = Yp-Xp;
      Lspd = Yp;
  }

else if(pwmX<(midpoint-midbuffer)){

      Xp = midpoint-pwmX;
      Xp = map(Xp, 0, pwmRange, 0, 255);
      Xp = constrain(Xp, 0, 255);
      Xp = Xp/Turnmod;
      
      Lspd = Yp-Xp;
      Rspd = Yp;
      
  }

      Rspd = constrain(Rspd, 0, 255); 
      Lspd = constrain(Lspd, 0, 255);

      lcd.setCursor(5, 1);
      lcd.print("R:");
      lcd.print(Rspd);
      lcd.print(" ");
      lcd.setCursor(11, 1);
      lcd.print("L:");
      lcd.print(Lspd);

      analogWrite(enA, Rspd);
      analogWrite(enB, Lspd);
 
      done1 = false; 

      delay(10);
      


      if (!done1) 
        return;
  } 

}

void Objdetect(){
  PingL();
  PingR();
  var1=0;
    if(distanceA>thresh and distanceB>thresh){
      LEDoff();
    }

    if(distanceA<thresh or distanceB<thresh){
  
    if((abs(distanceA-distanceB)<Sensitivity)){
      var1=3;
      Serial.print("Both Triggered  :  ");
      LEDBLUE();
    }
    else if(distanceA<distanceB){
      var1 = 1;
      Serial.print("Left triggered  :  ");
      LEDRED(); 
    }
    else if(distanceA>distanceB){
      var1=2;
      Serial.print("Right triggered :  ");
      LEDGREEN();
    }

    else if(distanceA>thresh or distanceB>thresh){
      var1=0;
      Serial.print("no trigger      :  ");
      LEDoff();
    }
  } 
  
  switch(var1){
    case 1:
      Serial.print("LEFT OBJECT DETECT PROTOCOL  : ");      
      Serial.print("Done : ");
      delay(50);
    break;
    case 2:
      Serial.print("RIGHT OBJECT DETECT PROTOCOL : ");  
      Serial.print("Done : ");
      delay(50);
    break;
    case 3:
      Serial.print("DIRECT OBJECT DETECT PROTOCOL: ");
      Serial.print("Done : ");
      delay(50);
    break;
    default:
      Serial.print("Forward Drive - Default ---  : ");
      Serial.print("     : ");
//      FWD();
//      analogWrite(enA, motspeed);
//      analogWrite(enB, motspeed);
      delay(50);
    break;
  }
}
