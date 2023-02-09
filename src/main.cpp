#include <M5StickCPlus.h>
#include "UNIT_ENV.h"
#include <stdio.h>
#include <stdbool.h>

SHT3X sht30;
QMP6988 qmp6988;

float nejnizsitlak = 0.0;
float nejvyssitlak = 0.0;
float aktualnitlak = 0.0;
float rozdiltlaku = 0.0;
float tolerancetlaku = 0.0;
int hps = 0;
int hpf = 0;
int fontVelikost = 1.85;
bool kali = true;


void kalibrace()
{
  M5.lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.lcd.fillScreen(BLACK);
  M5.lcd.setCursor(0,10);
  M5.Lcd.printf(" zmacknete tlacitka\r\n M5 kdyz stojite a\r\n pote postranni kde\r\n jste v podrepu\r\n pro kalibraci");
  while(1)
  {
    M5.BtnA.read();
    if (M5.BtnA.isPressed())
    {
      nejnizsitlak = qmp6988.calcPressure();
      hpf = nejnizsitlak;
    }
    M5.BtnB.read();
    if (M5.BtnB.isPressed())
    {   
      nejvyssitlak = qmp6988.calcPressure();
      hps = nejvyssitlak;
    }
    if (nejnizsitlak > nejvyssitlak)
    {
      continue;
    }
    if(nejnizsitlak > 0)
    {
      if (nejvyssitlak > 0)
      {
        rozdiltlaku = hps - hpf;
        if (rozdiltlaku < 0)
        {
          rozdiltlaku * (-1);
          tolerancetlaku = rozdiltlaku * 0.25;
          round(tolerancetlaku);
          return;
        }
        if (rozdiltlaku > 0)
        {
          tolerancetlaku = rozdiltlaku * 0.25;
          round(tolerancetlaku);
          return;
        }

      }
    } 
  }
  Serial.printf("konec");
}
void kalibrace1()
{
  while(1)
  {
    M5.BtnA.read();
    if (M5.BtnA.isPressed())
    {
      nejnizsitlak = qmp6988.calcPressure();
      hpf = nejnizsitlak;
    }
    M5.BtnB.read();
    if (M5.BtnB.isPressed())
    {   
      nejvyssitlak = qmp6988.calcPressure();
      hps = nejvyssitlak;
    }
    if (nejnizsitlak > nejvyssitlak)
    {
      continue;
    }
    if(nejnizsitlak > 0)
    {
      if (nejvyssitlak > 0)
      {
        rozdiltlaku = hps - hpf;
        if (rozdiltlaku < 0)
        {
          rozdiltlaku * (-1);
          tolerancetlaku = rozdiltlaku * 0.85;
          round(tolerancetlaku);
          return;
        }
        if (rozdiltlaku > 0)
        {
          tolerancetlaku = rozdiltlaku * 0.85;
          round(tolerancetlaku);
          return;
        }

      }
    } 
  }
}
void stats()
{
  M5.lcd.setCursor(0,18);
  M5.Lcd.printf("nejvyssi tlak: %.0f \n",nejvyssitlak);
  M5.lcd.setCursor(0,5);
  M5.Lcd.printf("nejnizsi tlak: %.0f \n",nejnizsitlak);
  M5.lcd.setCursor(0,30);
  M5.Lcd.printf("rozdil tlaku: %.0f \n", rozdiltlaku);
  M5.lcd.setCursor(0,40);
  M5.Lcd.printf("tolerance tlaku: %.0f \n", tolerancetlaku);
  M5.lcd.setCursor(0,80);
  M5.Lcd.printf("aktualni tlak: %.0f \n", aktualnitlak);
}
void setup() {
  M5.begin(); //Init M5Stack.  初始化M5Stack
  //M5.Power.begin(); //Init power  初始化电源模块
  Serial.begin(115200);
  M5.lcd.setRotation(1);
  Wire.begin(); //Wire init, adding the I2C bus.  Wire初始化, 加入i2c总线
  qmp6988.init();
  //M5.lcd.println(F("ENV Unit III test"));
}
void loop()
{
  M5.lcd.setTextSize(fontVelikost);
  if(kali)
  {
    kalibrace();
    kali = false;
  }
  kalibrace1();
  M5.Lcd.setTextColor(BLUE);
  M5.lcd.fillScreen(BLACK);
  aktualnitlak = qmp6988.calcPressure();
  stats();
  Serial.println(tolerancetlaku);
  if (aktualnitlak != nejvyssitlak||aktualnitlak != nejnizsitlak) 
  {
    M5.lcd.setCursor(0,50);
    M5.Lcd.printf("jsem v pohybu");
  }
  if (aktualnitlak - tolerancetlaku == nejnizsitlak)
  {
    M5.lcd.setCursor(0,75);
    M5.Lcd.printf("jsi v dřepu, dřep je hotov");
    delay(1000);
  }
  if (aktualnitlak + tolerancetlaku == nejvyssitlak)
  {
    M5.lcd.setCursor(0,75);
    M5.Lcd.printf("jsi v pod dřepu, poslední funguje");
    delay(1000);

  }
  if (nejvyssitlak - rozdiltlaku ) 
  delay(800);
}