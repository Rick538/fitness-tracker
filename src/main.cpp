#include <M5StickCPlus.h>
#include "UNIT_ENV.h"
#include <stdio.h>
#include <stdbool.h>
#include <WiFi.h>
#include <HTTPClient.h>

SHT3X sht30;
QMP6988 qmp6988;

const char* ssid = "tksteti";
const char* password = "ProsimTeNevim";

const char* server = "api.thingspeak.com";
const String apiKey = "YH8H47Z47TJYKD7Y";
const String channelID = " 2044897";

float nejnizsitlak = 0.0;
float nejvyssitlak = 0.0;
float aktualnitlak = 0.0;
float rozdiltlaku = 0.0;
float tolerancetlaku = 0.0;
int hps = 0;
int hpf = 0;
int fontVelikost = 1.85;
bool kali = true;
bool start = true;
bool middle = false;
bool end = false;
int drep = 0;
float kalorie = 0.0;
float cas_za_drep = 0.0;
float caz_za_cviceni = 0.0; 



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
    if (nejnizsitlak >= nejvyssitlak)
    {
      continue;
    }
    if(nejnizsitlak > 0 || nejvyssitlak > 0)
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
  M5.lcd.setCursor(0,100);
  M5.Lcd.printf("cas: %.0f \n", cas_za_drep);
}
WiFiClient client;
void setup() {
  M5.begin(); //Init M5Stack.  初始化M5Stack
  //M5.Power.begin(); //Init power  初始化电源模块
  Serial.begin(115200);
  M5.lcd.setRotation(1);
  Wire.begin(); //Wire init, adding the I2C bus.  Wire初始化, 加入i2c总线
  qmp6988.init();
  //M5.lcd.println(F("ENV Unit III test"));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");
}
}
void loop()
{
  M5.lcd.setTextSize(fontVelikost);
  if(kali)
  {
    kalibrace();
    kali = false;
  }
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
  if (nejnizsitlak + tolerancetlaku >= aktualnitlak)
  {
    if (start)
    {
      zacatek = millis();
      M5.lcd.setCursor(0,75);
      M5.Lcd.printf("jsi v dřepu, dřep je hotov");
      middle = true;
    }
    if (end)
    {
      konec = millis();
      kalorie = kalorie + 0.5;
      drep = drep + 1;
      M5.Lcd.setCursor(0,75);
      M5.Lcd.printf("jste na konci, udělali jste drep");
      String url = "http://" + String(server) + "/update?api_key=" + apiKey +
              "&field1=" + String(drep) +
              "&field2=" + String(kalorie) +
              "&field3=" + String(cas_za_drep);
       Serial.println(url);
  
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
      if (drep >= 10)
      {
        cas_za_drep = (start + konec)/1000;
        drep = 0;
        kalorie = 0.0;
        nejnizsitlak = 0.0;
        nejvyssitlak = 0.0;
        aktualnitlak = 0.0;
        kalibrace();
      }
      start = true;
      middle = false;
      end = false;
    }
  }
  if (nejvyssitlak - tolerancetlaku <= aktualnitlak)
  {
    if (middle)
    {
      M5.lcd.setCursor(0,75);
      M5.Lcd.printf("jsi v pod dřepu, poslední funguje");
      end = true;
      start = false;
    }
  }
  if (nejvyssitlak - rozdiltlaku ) 
  delay(800);
}