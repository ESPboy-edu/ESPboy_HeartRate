/*
ESPboy Heart Rate monitor with MAX30105 sensor
for www.ESPboy.com project by RomanS
https://hackaday.io/project/164830-espboy-games-iot-stem-for-education-fun
v1.0
*/

#include "ESPboyInit.h"
#include "ESPboyLED.h"
#include "MAX30105.h" 
#include "heartRate.h"

#define RATE_SIZE 8
#define DRAW_DELAY_STEPS 50
#define DRAW_DELAY_TIME 10

ESPboyInit myESPboy;
ESPboyLED myLED;
MAX30105 particleSensor;

int32_t irValues[128];

void setup(){
  Serial.begin(115200);
  memset(&irValues[0], 0, sizeof(irValues));
  myESPboy.begin("Heart Rate");
  myLED.begin(&myESPboy.mcp);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)){ 
    myESPboy.tft.setTextColor(TFT_RED);
    myESPboy.tft.drawString(F("Module not found"),16,120);
    while (1) delay(100);
  }
 
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  myESPboy.tft.drawLine(0,64,128,64,TFT_BLUE);
}


void loop(){
  static int32_t irValue, delta, irValueAver;
  static uint32_t rates[RATE_SIZE]; //Array of heart rates
  static uint32_t rateSpot = 0;
  static uint32_t lastBeat = 0; //Time at which the last beat occurred
  static float beatsPerMinute;
  static uint16_t beatAvg;
  static uint8_t ratecount=0;
  static int32_t drawValues[128];
  static uint32_t drawCountSteps=0;
  static uint32_t drawCountTime=0;
  static uint32_t temper, tmp[30];

 irValue = particleSensor.getIR();

 for(uint8_t i=0; i<29; i++) tmp[i]=tmp[i+1];
 tmp[29]=(uint32_t)(particleSensor.readTemperature()*10);

 temper=0;
 for(uint8_t i=0; i<30; i++) temper+=tmp[i];
 temper/=30;
 
 myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
 if (temper>365) myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
 if (temper>369) myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
 if (temper>380) myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);
 String toPrint = "t"+(String)((float)temper/10)+"C  ";
 myESPboy.tft.drawString(toPrint,85,0);
   
 if (checkForBeat(irValue)){
  myLED.setRGB(100,0,0);
  tone(D3,50,50);
  delta = millis() - lastBeat;
  lastBeat = millis();
  beatsPerMinute = (float)60 / ((float)delta / 1000.0);
 
  if (beatsPerMinute < 250 && beatsPerMinute > 35){
    rates[rateSpot++] = (uint32_t)beatsPerMinute; //Store this reading in the array
    rateSpot %= RATE_SIZE; //Wrap variable
 
    beatAvg = 0;
    for (uint8_t x = 0 ; x < RATE_SIZE; x++) beatAvg += rates[x];
    beatAvg /= RATE_SIZE;

   if (ratecount<RATE_SIZE) ratecount++;
   }
}


 
if (irValue < 60000 ){
  myESPboy.tft.setTextColor(TFT_RED);
  myESPboy.tft.drawString(F("Place index finger on"),0,112);
  myESPboy.tft.drawString(F("the sensor"),32,120);

  myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
  myESPboy.tft.drawString("t--.--C",85,0);

  myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
  myESPboy.tft.setTextSize(3);
  myESPboy.tft.drawString("--  ",0,0);
  myESPboy.tft.setTextSize(1);
  
  myLED.setRGB(0,0,10);

  tone(D3,100,100);
  delay(100);
  tone(D3,200,100);
  
  ratecount=0;
  drawCountSteps=0;
  while(particleSensor.getIR()<50000) delay(200);
  myESPboy.tft.setTextColor(TFT_BLACK);
  myESPboy.tft.drawString(F("Place index finger on"),0,112);
  myESPboy.tft.drawString(F("the sensor"),32,120);
}
else{
  if (ratecount<RATE_SIZE) myESPboy.tft.setTextColor(TFT_BLUE, TFT_BLACK);
  else myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if (beatAvg>100) myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  if (beatAvg>120) myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);
  myESPboy.tft.setTextSize(3);
  myESPboy.tft.drawString((String)beatAvg+"  ",0,0);
  myESPboy.tft.setTextSize(1);
}

 if(myLED.getRGB())myLED.setRGB(0,0,0);

 memmove(&irValues[0], &irValues[1], 127*sizeof(int32_t));
 irValues[127] = irValue;
 
 if(drawCountSteps<DRAW_DELAY_STEPS) drawCountSteps++;
 
 if (millis()-drawCountTime>DRAW_DELAY_TIME && drawCountSteps>=DRAW_DELAY_STEPS){
   drawCountTime = millis(); 
   for(uint8_t i=0; i<128; i++) 
     myESPboy.tft.drawPixel(i,128-((drawValues[i]-irValueAver)/20+64), TFT_BLACK);
   irValueAver=0;
   for(uint8_t i=20; i<120; i++) 
     irValueAver+=irValues[i];
   irValueAver/=100;
   myESPboy.tft.drawLine(0,64,128,64,TFT_BLUE);
   for(uint8_t i=0; i<128; i++) 
     myESPboy.tft.drawPixel(i,128-((irValues[i]-irValueAver)/20+64), TFT_GREEN);

   memcpy(&drawValues[0], &irValues[0], sizeof(irValues));
 }
}
