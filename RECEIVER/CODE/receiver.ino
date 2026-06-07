#include <Wire.h>

#define TCA_ADDR 0x70   // Dirección del TCA9548A
#define  MCP_ADDR 0x60  // Dirección del DAC

int b1 = 0; //maza 1
int b2 = 0;  //maza2
int b3 = 0;  //exciter1
int b4 = 0;  //exciter2
float v1 = 0; //valor resp 0-100
String data = "";

// Seleccionar canal del TCA9548A
void tcaSelect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << i);  // Activa canal i
  Wire.endTransmission();
}

void writeDAC(uint16_t value) {
  Wire.beginTransmission(MCP_ADDR);
  // Comando rápido para MCP4725
  Wire.write(0x40);  // Write DAC register
  Wire.write(value >> 4);        // 8 bits altos
  Wire.write((value & 0xF) << 4); // 4 bits bajos
  Wire.endTransmission();
}

void setup() {

   Serial.begin(38400);
   delay(1000);
   Wire.begin();
   
  pinMode(2, OUTPUT); // Setup for maza1
  pinMode(3, OUTPUT); // Setup for maza2

}


void loop() {
  while (Serial.available()) 
  {

  char c = Serial.read();
  
    if (c == '\n') {

      // Parsear
      int valores[5];

      int index = 0;
      char buffer[50];
      data.toCharArray(buffer, 50);

      char *token = strtok(buffer, " ");

      while (token != NULL && index < 5) {
        valores[index++] = atoi(token);
        token = strtok(NULL, " ");
      }
         
      b1 = valores[0];
      b2 = valores[1];
      b3 = valores[2];
      b4 = valores[3];
      if(valores[4] <0){v1 = -valores[4];}
      if(valores[4] >=0){v1 = valores[4];}    
      data = "";
    }
    else {
      data += c;
    }
  }

//// MAZAS
if (b1==1){digitalWrite(2,HIGH);}
if (b1==0){digitalWrite(2,LOW);}
if (b2==1){digitalWrite(3,HIGH);}
if (b2==0){digitalWrite(3,LOW);}

//// EXCITERS
if (b3==1){tcaSelect(2);writeDAC(10);} //freq}
if (b3==0){tcaSelect(2);writeDAC(3600);}
if (b4==1){tcaSelect(1);writeDAC(10);}
if (b4==0){tcaSelect(1);writeDAC(3600);}

int amplitud=map(int(v1), 0, 2000, 3400, 0); //valor mapeado a DAC 0=3600

if(amplitud>3400){amplitud=3400;}
if(amplitud<0){amplitud=0;}

tcaSelect(0);writeDAC(amplitud);
tcaSelect(3);writeDAC(amplitud);

delay(3);
      
}

