#include <M5StickCPlus.h>
#include "UNIT_ENV.h"
#include <stdio.h>
#include <stdbool.h>
#include <WiFi.h>
#include <HTTPClient.h>

SHT3X sht30;
QMP6988 qmp6988;

const char* ssid = "Beranovi 2,4GHZ";
const char* password = "Beran 58";
const String apiKey = "YH8H47Z47TJYKD7Y";
const String channelID = "2044897";

float nejnizsitlak = 0.0;
float nejvyssitlak = 0.0;
float aktualnitlak = 0.0;
float rozdiltlaku = 0.0;
float tolerancetlaku = 0.0;
int hps = 0;
int hpf = 0;
int fontVelikost = 1.85;
float zacatek = 0.0;
float konec = 0.0;
bool kali = true;
bool start = false;
bool middle = false;
bool end = false;
int drep = 8;
float kalorie = 0.0;      
float cas_za_drep = 0.0;   
float cas_za_cviceni = 0.0;                                                       

void kalibrace()
{
  M5.lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.lcd.fillScreen(BLACK);
  M5.lcd.setCursor(0,5);
  M5.Lcd.printf(" zmacknete tlacitka\r\n M5 kdyz stojite a\r\n pote postranni kde\r\n jste v podrepu\r\n pro kalibraci\r\n v kazde fazi drepu\r\n budte alespon\r\n 1 vterinu");
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
    if(nejnizsitlak > 0||nejvyssitlak > 0)
    {
      rozdiltlaku = hps - hpf;
      if (rozdiltlaku < 0)
      {
        rozdiltlaku * (-1);
        tolerancetlaku = rozdiltlaku * 0.15;
        round(tolerancetlaku);
        start = true;
        return;
      }
      if (rozdiltlaku > 0)
      {
        tolerancetlaku = rozdiltlaku * 0.15;
        round(tolerancetlaku);
        start = true;
        return;
      }
    }
  }
}
void konec_hodnoty()
{
  M5.lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE); 
  M5.lcd.fillScreen(BLACK);
  M5.lcd.setCursor(0,5);
  M5.Lcd.printf("drepy: ");
  M5.lcd.setCursor(85,5);
  M5.Lcd.println(drep);
  M5.lcd.setCursor(0,50);
  M5.Lcd.printf("kalorie: ");
  M5.lcd.setCursor(105,50);
  M5.Lcd.println(kalorie);
  delay(5000);
}
void stats()
{
  M5.lcd.setCursor(0,5);
  M5.Lcd.printf("nejnizsi tlak: %.0f \n",nejnizsitlak);
  M5.lcd.setCursor(0,15);
  M5.Lcd.printf("nejvyssi tlak: %.0f \n",nejvyssitlak);
  M5.lcd.setCursor(0,25);
  M5.Lcd.printf("aktualni tlak: %.0f \n", aktualnitlak);
  M5.lcd.setCursor(0,35);
  M5.Lcd.printf("rozdil tlaku: %.0f \n", rozdiltlaku);
  M5.lcd.setCursor(0,45);
  M5.Lcd.printf("tolerance tlaku: %.0f \n", tolerancetlaku);
  M5.lcd.setCursor(0,65);
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
  M5.begin();
  M5.Lcd.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Lcd.print(".");
  }
  M5.Lcd.print("");
  M5.Lcd.print("Wi-Fi connected!");
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
  if (nejnizsitlak + tolerancetlaku >= aktualnitlak)
  {
    if (start)
    {
      zacatek = millis();
      Serial.println(zacatek);
      M5.lcd.setCursor(0,75);
      M5.lcd.printf("jste na zacatku, následuje pohyb dolu");
      middle = true;
    }
    if (end)
    {
      konec = millis();
      Serial.println(konec);
      kalorie = kalorie + 0.5;
      drep = drep + 1;
      cas_za_drep = (konec - start)/1000;
      Serial.println(cas_za_drep);
      M5.lcd.setCursor(0,75);
      cas_za_cviceni = cas_za_cviceni + cas_za_drep;
      if (drep >= 10)
      {
        M5.lcd.printf("jste na konci, udelal jste drep\r\n resetovali se hodnoty, muzete pokracovat");
        String url = "http://api.thingspeak.com/update?api_key=" + apiKey + "&field1=" + String(cas_za_drep)+ "&field2=" + String(kalorie);
        HTTPClient http;
        http.begin(client, url);
        int httpCode = http.GET();
        if (httpCode > 0) {
          Serial.println("Data sent to ThingSpeak");
        } else {
          Serial.println("Error sending data to ThingSpeak");
        }
        http.end();
        konec_hodnoty();
        drep = 0;
        kalorie = 0;
        nejnizsitlak = 0.0;
        aktualnitlak = 0.0;
        nejvyssitlak = 0.0;
        delay(2000);
        kalibrace();
      }
      start = true;
      middle = false; // toto je aby se předešlo nějakým problémům 
      end = false; 
    }
  }
  else 
  {
    M5.lcd.setCursor(0,60);
    M5.Lcd.printf("jste v pohybu");
  }
  if (nejvyssitlak - tolerancetlaku <= aktualnitlak)
  {
    if(middle)
    {
      M5.lcd.setCursor(0,75);
      M5.lcd.printf("jste uprosted drepu, následuje pohyb nahoru");
      end = true;
      start = false;
    }
  }
  else
  {
    M5.lcd.setCursor(0,60);
    M5.Lcd.printf("jste v pohybu");
  }
  delay(800);
}