#include <M5StickCPlus.h>
#include "UNIT_ENV.h"
#include <stdio.h>
#include <stdbool.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <password.h>

QMP6988 qmp6988;

const char* ssid = jmeno;
const char* password = heslo;
const char* server = web;
const String apiKey = klic;
const String channelId = kanal;

float NejnizsiTlak = 0.0;
float NejvyssiTlak = 0.0;
float AktualniTlak = 0.0;
float RozdilTlaku = 0.0;
float ToleranceTlaku = 0.0;
int Hps = 0;
int Hpf = 0;
int FontVelikost = 1.85;
bool Kali = true;
bool Start = false;
bool Middle = false;
bool End = false;
bool Time = false;
int Drep = 0;
float Kalorie = 0.0;
float CasZaCviceni = 0.0;
float UplynulyCas = 0.0;
bool Timer = false;
time_t TimeStart, TimeEnd;

void kalibrace()
{
  M5.lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.lcd.fillScreen(BLACK);
  M5.lcd.setCursor(0,5);
  M5.Lcd.printf("pro kalibraci \r\nzmacknete tlacitka \r\nM5 kdyz stojite\r\npostranni v drepu\r\nvyckejte vzdy \r\n1 vterinu \r\nprosim nepospichejte");
  while(1)
  {
    M5.BtnA.read();
    if (M5.BtnA.isPressed())
    {
      NejnizsiTlak = qmp6988.calcPressure();
      Hpf = NejnizsiTlak;
    }
    M5.BtnB.read();
    if (M5.BtnB.isPressed())
    {   
      NejvyssiTlak = qmp6988.calcPressure();
      Hps = NejvyssiTlak;
    }
    if (NejnizsiTlak <= 0)
    {
      continue;
    }
    if (NejnizsiTlak >= NejvyssiTlak)
    {
      continue;
    }
    if(NejnizsiTlak > 0|| NejvyssiTlak > 0)
    {
      RozdilTlaku = Hps - Hpf;
      if (RozdilTlaku < 8)
      {
        NejnizsiTlak = 0.0;
        AktualniTlak = 0.0;
        NejvyssiTlak = 0.0;
        RozdilTlaku = 0.0;
        continue;
      }
      if (RozdilTlaku >= 8)
      {
        ToleranceTlaku = RozdilTlaku * 0.15;
        round(ToleranceTlaku);
        Start = true;
        Timer = true;
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
  M5.Lcd.println(Drep);
  M5.lcd.setCursor(0,50);
  M5.Lcd.printf("kalorie: ");
  M5.lcd.setCursor(105,50);
  M5.Lcd.println(Kalorie);
  delay(5000);
}
void stats()
{
  M5.lcd.setCursor(0,5);
  M5.Lcd.printf("nejnizsi tlak: %.0f \n",NejnizsiTlak);
  M5.lcd.setCursor(0,15);
  M5.Lcd.printf("nejvyssi tlak: %.0f \n",NejvyssiTlak);
  M5.lcd.setCursor(0,25);
  M5.Lcd.printf("aktualni tlak: %.0f \n", AktualniTlak);
  M5.lcd.setCursor(0,35);
  M5.Lcd.printf("rozdil tlaku: %.0f \n", RozdilTlaku);
  M5.lcd.setCursor(0,45);
  M5.Lcd.printf("tolerance tlaku: %.0f \n", ToleranceTlaku);
  M5.lcd.setCursor(0,65);
  M5.Lcd.printf("drepy: ");
  M5.lcd.setCursor(35,65);
  M5.Lcd.println(Drep);

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
  M5.Lcd.printf("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Lcd.printf(".");
  }
  M5.Lcd.printf("");
  M5.Lcd.printf("Wi-Fi connected!");
}
void loop()
{
  M5.lcd.setTextSize(FontVelikost);
  if(Kali)
  {
    kalibrace();
    Kali = false;
  }
  M5.Lcd.setTextColor(BLUE);
  M5.lcd.fillScreen(BLACK);
  AktualniTlak = qmp6988.calcPressure();
  stats();
  if (NejnizsiTlak + ToleranceTlaku >= AktualniTlak)
  {
    if (Start == true)
    {
      M5.lcd.setCursor(0,85);
      M5.lcd.printf("jste na zacatku \r\nnásleduje pohyb dolu");
      Middle = true;
      if (Timer == true)
      {
        TimeStart = time(NULL);
        Timer = false;
      }
    }
    if (End == true)
    {

      Drep ++;
      M5.lcd.setCursor(0,85);
      M5.lcd.printf("drep hotov \r\nvyckejte 1 vterinu");
      if (Drep >= 5)
      {
        TimeEnd = time(NULL);
        CasZaCviceni = difftime(TimeEnd, TimeStart);
        Kalorie = ((6 * 60 * 0.0175)/60)*CasZaCviceni;
        M5.lcd.setCursor(0,85);
        M5.lcd.printf("jste na konci celého cvičení \r\nresetovali se hodnoty \r\nmuzete pokracovat");
        String url = "http://api.thingspeak.com/update?api_key=" + apiKey + "&field1=" + String(CasZaCviceni) + "&field2=" + String(Kalorie) + "&field3=" + String(Drep);
        HTTPClient http;
        http.begin(client, url);
        int httpCode = http.GET();
        if (httpCode > 0) 
        {
          Serial.println("Data byla odeslána na ThingSpeak");
        } else {
          Serial.println("Data se neposlala na ThingSpeak");
        }
        http.end();
        konec_hodnoty();
        Drep = 0;
        Kalorie = 0;
        NejnizsiTlak = 0.0;
        AktualniTlak = 0.0;
        NejvyssiTlak = 0.0;
        delay(2000);
        Kali = true;
      }
      Start = true;
      Middle = false; // toto je aby se předešlo nějakým problémům
      End = false;
    }
  }
  else 
  {
    M5.lcd.setCursor(150,85);
    M5.Lcd.printf("jste v pohybu");
  }
  if (NejvyssiTlak - ToleranceTlaku <= AktualniTlak)
  {
    if (Middle == true)
    {
      M5.lcd.setCursor(0,85);
      M5.lcd.printf("jste uprosted drepu\r\n následuje pohyb nahoru");
      End = true;
      Start = false;
    }
  }
  else
  {
    M5.lcd.setCursor(150,85);
    M5.Lcd.printf("jste v pohybu");
  }
  delay(800);
}