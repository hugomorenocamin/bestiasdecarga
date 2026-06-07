#include <Filters.h>

#include <AH/Timing/MillisMicrosTimer.hpp>
#include <Filters/Butterworth.hpp>

auto filter = butter<2>(.01);
auto filter2 = butter<2>(.14);

int Rtrig;
char Rstr[5];


const int ecgPin = A0;
const int respPin = A1;

// Buffer para filtro de 5 muestras ECG
float x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0;

float filtered = 0; //señal filtrada ECG
float filteredy = 0; //señal filtrada RESP

// Derivada ECG
float prevFiltered = 0;
float slope = 0;

// Derivada RESP
float prevFilteredy = 0;
float slopey = 0.0;
float respp=0;

// Umbrales ECG
float slopeThresholdR = 15;   // pendiente fuerte (R)
float slopeThresholdT = 3;    // pendiente más suave (T)

unsigned long currentTime = 0;
unsigned long dt = 0;
unsigned long lastRTime = 0;
unsigned long lastTTime = 0;

bool detectedR = false;
bool detectedT = false;

int maza1=0;
int maza2=0;
int R=0;
int T=0;

int exciter1=0;
int exciter2=0;
float bandamax=500;
float bandamin=200;


void setup() {
  Serial.begin(38400);
  pinMode(2, INPUT); // Setup for leads off detection LO +
  pinMode(3, INPUT); // Setup for leads off detection LO -
  pinMode(6, INPUT_PULLUP); // interruptor RF
  pinMode(10, OUTPUT); // Setup for led resp up
  pinMode(11, OUTPUT); // Setup for led resp down
  pinMode(12, OUTPUT); // Setup for led maza1
  pinMode(13, OUTPUT); // Setup for led maza2

}

void loop() {
  
  currentTime = millis();
  
//////////    procesa ECG   /////////
  if((digitalRead(2) == 0) && (digitalRead(3) == 0))
  {
  // Leer nueva muestra
 
  // --- FILTRO BUTTERW DE 35Hz/250Hz->0.14 ---
  filtered = filter2(analogRead(ecgPin));
  
  // --- DERIVADA (pendiente) ---
  slope = filtered - prevFiltered;

  // --- DETECCIÓN DE ONDA R ---
  // busco subida rápida (pendiente positiva grande)
  if (slope > slopeThresholdR && detectedR==false) {
    lastRTime = currentTime;
    detectedR = true;      
    maza1=1;
    digitalWrite(12,HIGH);   
  }

  if(detectedR  && (currentTime-lastRTime)>100) //apago maza1 pasados 100ms
  {
        maza1=0;   
        digitalWrite(12,LOW);         
  }

  // --- DETECCIÓN DE ONDA T ---
  if (detectedR ) {
    dt = currentTime - lastRTime;

    // ventana fisiológica típica
    if (dt > 150 && dt < 400 &&  detectedT == false) {
      if (slope > slopeThresholdT) {
         
         detectedT = true;
         lastTTime = currentTime;
         maza2=1;
         digitalWrite(13,HIGH);       
       }
    }
  }
 
    if(detectedT==true && (currentTime-lastTTime) >100) //apago maza2
    {
      maza2=0;
      digitalWrite(13,LOW);
      detectedT=false;
      detectedR = false;
    }

    // reset de seguridad
    if (dt > 500) {
      detectedR = false;
      detectedT = false;
      maza1=0;
      maza2=0;  

    }
  
  prevFiltered = filtered;
  }

  else{
   ;
  }
  /////////////////// fin ECG  ///////////////

 
  ////////////detecto respiracion //////////

  // --- FILTRO BUTTERW DE 2.5Hz/250Hz->0.01 ---
  filteredy=filter(analogRead(respPin));
  // --- DERIVADA (pendiente) ---
  slopey =  filteredy-prevFilteredy;

 
  if (slopey>.2){exciter1=1;analogWrite(10, 10);}
  if (slopey<-0.2){exciter2=1;analogWrite(11, 10);}
  if (slopey>=-0.2 && slopey<=0.2){exciter1=0; exciter2=0;}

  if(exciter1==0){analogWrite(10, 0);}
  if(exciter2==0){analogWrite(11, 0);}

  prevFilteredy = filteredy;

  delay(5); // ~250 Hz

  //valor en % de respiracion
  respp= mapf(filteredy,bandamin,bandamax,0.0,100.0);
  if (respp<0.0){respp=0.0;}
  if (respp>100.0){respp=100.0;}

  if(digitalRead(6) == 1)
    {

    Serial.print(maza1);
    Serial.print(" ");
    Serial.print(maza2);
    Serial.print(" "); 
    Serial.print(exciter1);
    Serial.print(" ");
    Serial.print(exciter2);
    Serial.print(" ");
    Serial.println(slopey*1000);

  }      
      
}

float mapf(float x,float in_min,float in_max,float out_min,float out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}