#include <TFT_eSPI.h>
#include "PCF85063A-SOLDERED.h"
#include "Slider-potentiometer-easyC-SOLDERED.h"
#include "BMP180-SOLDERED.h"
#include "font.h"
#include "INA219.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

PCF85063A rtc; 
INA219 INA(0x40);
sliderPot slider;
Bmp_180 myBmp;

#define back 0x9D91
#define bigFont DSEG7_Modern_Bold_64
#define small DejaVu_Sans_Mono_Bold_12
#define middle Monospaced_bold_18
#define middle2 DSEG7_Classic_Bold_30
#define clockFont DSEG7_Modern_Bold_20
#define dc Cousine_Regular_38
#define color1 TFT_BLACK
//#define color1 0x18E3

float cur=0;  //curent
float vol=0;  //voltage
float power=0; // power
float averageVoltageTemp=0; 
float averageVoltage=0;  //average voltage
float aC=0;   //averagge current
float aCt=0;
int nReadings=50;
int value=9660;
double temperature=0;
bool invert=false;

int h,m,s,ts, sBlock=0;
String hour="";
String minute="";

int bright=4;
int brightnesses[7]={35,70,105,140,175,210,250};
int deb=0;
int c=0;
int pot=0;

void setup() 
{  
  pinMode(0,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);
  Wire.begin(43,44);
  if (!INA.begin() )
  {
    Serial.println("could not connect. Fix and Reboot");
  }
  INA.setMaxCurrentShunt(5, 0.1);
  rtc.begin();  
  slider.begin();
  myBmp.begin();
    
  pinMode(0,INPUT_PULLUP);
  tft.init();
  tft.setRotation(1);
  sprite.createSprite(320,170);
  sprite.setTextColor(color1,back);
  sprite.setFreeFont(&bigFont);

     ledcSetup(0, 10000, 8);
     ledcAttachPin(38, 0);
     ledcWrite(0, brightnesses[bright]);  //brightnes of screen
}

void draw()
  {
     sprite.setTextColor(color1,back);
     sprite.fillSprite(back);
     sprite.setFreeFont(&bigFont);
     sprite.setTextDatum(0);
     if(averageVoltage<10)
     sprite.drawFloat(averageVoltage,3,82,60);
     else
     sprite.drawFloat(averageVoltage,2,82,60);

     sprite.setFreeFont(&middle2);
     if(aC<0 || aC>100)
     sprite.drawFloat(aC,1,180,8);
     else
     sprite.drawFloat(aC,2,180,8);
 
       sprite.setFreeFont(&clockFont);
       sprite.drawString(hour+":"+minute,4,36);
       sprite.setFreeFont(&middle);
       sprite.setTextColor(back,color1);
       sprite.drawString(" AUTO  ",4,60);
       sprite.drawString(" POWER  ",90,6);
     
       sprite.setTextColor(color1,back);
       sprite.drawString("mA",288,8);
       sprite.drawString("V",298,60);
       sprite.drawString("TIME",4,6);
       //sprite.drawString("DC",4,80);
       sprite.drawString(String(temperature)+"C",4,108);
       
       sprite.drawString(String(power/1000)+"W",90,26);
       
         sprite.fillRect(90,46,224,2,color1);
         for(int i=0;i<sBlock;i++)
         sprite.fillRect(6+(i*8),26,6,5,color1);

      sprite.setFreeFont(&small);
      sprite.drawString("TEMP:",4,94);
      sprite.setTextDatum(4);

 int temp=pot;
 for(int i=0;i<80;i++)
 {
 if((temp%10)==0){
     sprite.drawLine(i*4,170,i*4,158,color1);
     sprite.drawLine((i*4)+1,170,(i*4)+1,158,color1);
     sprite.drawFloat(temp,0,i*4,148);}
 else if((temp%5)==0 && (temp%10)!=0)
 {sprite.drawLine(i*4,170,i*4,162,color1);
 sprite.drawLine((i*4)+1,170,(i*4)+1,162,color1);}
 else
 {sprite.drawLine(i*4,170,i*4,164,color1);}
 temp=temp+1;
 }
    sprite.fillTriangle(160,138,156,130,164,130,color1);
    sprite.pushSprite(0,0);
}


void loop() {
   pot=slider.getValue();
   bright=map(pot,0,1024,0,250);
   ledcWrite(0, bright);

  if(digitalRead(14)==0)
       {if(deb==0)
       {deb=1; 
       invert=!invert;
       tft.invertDisplay(invert);
       }
  }else deb=0;

  cur=INA.getCurrent_mA();
  vol=INA.getBusVoltage();
  
  h=rtc.getHour();
  m=rtc.getMinute();
  s=rtc.getSecond();
  if(s!=ts)
  {sBlock++; if(sBlock==9) sBlock=0; ts=s;}
  if(h<10) hour="0"+String(h); else hour=String(h);
  if(m<10) minute="0"+String(m); else minute=String(m);
    
  averageVoltageTemp=averageVoltageTemp+vol;
  aCt=aCt+cur;
  c++;
  if(c==nReadings)
  {
    averageVoltage=averageVoltageTemp/nReadings;
    aC=aCt/nReadings;
    aCt=0;
    averageVoltageTemp=0;
    c=0;
    int status = myBmp.startTemperature();
    myBmp.getTemperature(temperature);
  }
  power=(averageVoltage*(aC*1000.00))/1000.00;
  draw();
}
