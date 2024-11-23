#include <Wire.h> //Подключаем библиотеку для использования I2C интерфейса с модулем RTC
#include <avr/wdt.h> //Библиотека для работы со сторожевым таймером

#include <OneWire.h>
#include <DallasTemperature.h>

#include "RTClib.h" //Подключаем библиотеку для использования модуля часов реального времени RTC

#include <EEPROM.h>


OneWire oneWire(A3);
DallasTemperature ds(&oneWire);

#include <LiquidCrystal_I2C.h> // библиотека экрана
LiquidCrystal_I2C lcd(0x27, 20, 4);

int PWM_LW_MIN = 0; //Если необходим ток покоя на LED - изменить эту константу
int PWM_LW_MAX = 250; //Если необходимо ограничить максимальную яркость - уменьшить значение
#define PWM_LW_PIN 3 //Пин порта, где будет ШИМ LW

#define mn 60UL //Дополнительные константы для удобства
#define hr 3600UL //Отражают соответствующие количества секунд
#define d 86400UL

RTC_DS1307 RTC;

int  RH = 0;
int  RM = 0;
int  RD = 0;
int  ZH = 0;
int  ZM = 0;
int  ZD = 0;
int  PS = 0;
int  ScD = 250;
int  ScN = 50;


const int buttonPin = 2;
int buttonState = 0;
const int buttonPin2 = 4;
int buttonState2 = 0;
const int buttonPin3 = 5;
int buttonState3 = 0;
int pos = 1;


//********************************************************************************************

void setup() {

  Serial.begin(9600);
  Wire.begin(); //Инициируем I2C интерфейс

  RH = EEPROM.read(0);
  RM = EEPROM.read(1);
  RD = EEPROM.read(2);
  ZH = EEPROM.read(3);
  ZM = EEPROM.read(4);
  ZD = EEPROM.read(5);

  ds.begin();

  RTC.begin(); //Инициирум RTC модуль


  pinMode(buttonPin, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);

  lcd.init(); // запускаем библиотеку экрана
  lcd.backlight();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(2020, 1, 1, 12, 0, 0));
  }
  //RTC.adjust(DateTime(2012, 7, 31, 21, 53, 1));
  //RTC.adjust(DateTime(__DATE__, __TIME__));
} // КОНЕЦ ИНИЦИАЛИЗАЦИИ

//********************************************************************************************

void loop() // ПРОГРАММЫй безусловный ЦИКЛ
{


  long sunrise_start = RH * hr + RM * mn; //Начало восхода в 9 - 45
  long sunrise_duration = RD * mn; //Длительность восхода 30 минут
  long sunset_start = ZH * hr + ZM * mn; //начало заката в 21-15
  long sunset_duration = ZD * mn; //Длительность заката 30 минут

  long pwm_LW;
  DateTime myTime = RTC.now(); //Читаем данные времени из RTC при каждом выполнении цикла
  long Day_time = myTime.unixtime() % 86400; //сохраняем в переменную - время в формате UNIX

  if ((Day_time < sunrise_start) || //Если с начала суток меньше чем начало восхода
      (Day_time >= sunset_start + sunset_duration)) { //Или больше чем начало заката + длительность
    pwm_LW = PWM_LW_MIN; //Величина для записи в порт равна минимуму


    //*********************************************************************************************
    // обработка интервала восхода
    //*********************************************************************************************
  } else if ((Day_time >= sunrise_start) && //Если с начала суток больше чем начало восхода
             (Day_time < sunrise_start + sunrise_duration)) { //И меньше чем начало восхода + длительность

    pwm_LW = ((Day_time - sunrise_start) * (PWM_LW_MAX - PWM_LW_MIN)) / sunrise_duration; //Вычисляем для рассвета величину для записи в порт ШИМ


    //*********************************************************************************************
    // обработка интервала заката
    //*********************************************************************************************
  } else if ((Day_time >= sunset_start) && //Если начала суток больше чем начало заката и меньше чем
             (Day_time < sunset_start + sunset_duration)) { //начало заката плюс длительность

    pwm_LW = ((sunset_start + sunset_duration - Day_time) * (PWM_LW_MAX - PWM_LW_MIN)) / sunrise_duration; //Вычисляем для заката величину для записи в порт ШИМ

    //********************************************************************************************
    // обработка интервала от конца рассвета и до начала заката,
    // когда свет должен быть включен на максимальную яркость
    //********************************************************************************************
  } else {
    pwm_LW = PWM_LW_MAX; //Устанавливаем максимальную величину для записи в порт ШИМ

  }

  analogWrite(PWM_LW_PIN, pwm_LW); //Пишем в порт вычисленное значение



  if (Day_time >= sunrise_start && Day_time <= sunset_start ) {
    analogWrite(11, ScD);
  }
  else {
    analogWrite(11, ScN);
  }



  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    pos++;
    lcd.clear();
    delay (200);
  }


  if (pos >= 16) {
    lcd.clear();
    pos = 1;
  }

  switch (pos) {
    case 1:
      ds.requestTemperatures();
      //lcd.clear();
      lcd.setCursor(8, 1);
      lcd.print("t ");
      lcd.print(ds.getTempCByIndex(0));
      lcd.print("C");
      lcd.setCursor(0, 0);
      lcd.print("MINSK");
      lcd.setCursor(6, 0);
      if (myTime.day() < 10) lcd.print("0");
      lcd.print(myTime.day(), DEC);
      lcd.print("/");
      if (myTime.month() < 10) lcd.print("0");
      lcd.print(myTime.month(), DEC);
      lcd.print("/");
      lcd.print(myTime.year(), DEC);
      lcd.setCursor(0, 1);
      if (myTime.hour() < 10) lcd.print("0");
      lcd.print(myTime.hour(), DEC);
      lcd.print(":");
      if (myTime.minute() < 10) lcd.print("0");
      lcd.print(myTime.minute(), DEC);
      break;

    case 2:

      lcd.setCursor(1, 0);
      lcd.print("RASSVET START");
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        RH++;

        delay(200);
      }
      if (RH >= 24)  {
        RH = 0;
      }
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        RM++;

        delay(200);
      }
      if (RM >= 60)  {
        RM = 0;
      }
      lcd.setCursor(5, 1);
      if ( RH < 10) lcd.print("0");

      lcd.print(RH);
      lcd.print(":");
      if ( RM < 10) lcd.print("0");

      lcd.print(RM);
      break;

    case 3:

      lcd.setCursor(2, 0);
      lcd.print("RASSVET LONG");
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        RD--;
        delay(200);
      }
      if (RD > 60)  {
        RD = 0;
      }
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        RD++;
        delay(200);
      }
      if (RD < 0)  {
        RD = 60;
      }
      lcd.setCursor(7, 1);
      if ( RD < 10) lcd.print("0");
      lcd.print(RD);
      break;
    case 4:

      lcd.setCursor(3, 0);
      lcd.print("ZAKAT START");
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        ZH++;
        delay(200);
      }
      if (ZH >= 24)  {
        ZH = 0;
      }
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        ZM++;
        delay(200);
      }
      if (ZM >= 60)  {
        ZM = 0;
      }
      lcd.setCursor(6, 1);
      if ( ZH < 10) lcd.print("0");
      lcd.print(ZH);
      lcd.print(":");
      if ( ZM < 10) lcd.print("0");
      lcd.print(ZM);
      break;

    case 5:

      lcd.setCursor(3, 0);
      lcd.print("ZAKAT LONG");
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        ZD--;
        delay(100);
      }
      if (ZD > 60)  {
        ZD = 0;
      }
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        ZD++;
        delay(100);
      }
      if (ZD < 0)  {
        ZD = 60;
      }
      lcd.setCursor(7, 1);
      if ( ZD < 10) lcd.print("0");
      lcd.print(ZD);
      break;

    case 6:

      lcd.setCursor(0, 0);
      lcd.print("POWER LAMP");
      lcd.print(" ");
      PS = PWM_LW_MAX / 2.5;
      lcd.print(PS);
      lcd.print(" ");
      lcd.print("%");
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        PWM_LW_MAX = PWM_LW_MAX + 25;

        delay(200);
        lcd.clear();
      }
      if (PWM_LW_MAX > 250)  {
        PWM_LW_MAX = 250;

      }
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        PWM_LW_MAX = PWM_LW_MAX - 25;

        delay(200);
        lcd.clear();
      }
      if (PWM_LW_MAX < 0)  {
        PWM_LW_MAX = 0;
      }
      lcd.setCursor(3, 1);
      if (PS >= 10 && PS < 20 ) {
        lcd.print("*");
      }
      if (PS >= 20 && PS < 30 ) {
        lcd.print("**");
      }
      if (PS >= 30 && PS < 40 ) {
        lcd.print("***");
      }
      if (PS >= 40 && PS < 50 ) {
        lcd.print("****");
      }
      if (PS >= 50 && PS < 60 ) {
        lcd.print("*****");
      }
      if (PS >= 60 && PS < 70 ) {
        lcd.print("******");
      }
      if (PS >= 70 && PS < 80 ) {
        lcd.print("*******");
      }
      if (PS >= 80 && PS < 90 ) {
        lcd.print("********");
      }
      if (PS >= 90 && PS < 100 ) {
        lcd.print("*********");
      }
      if (PS >= 100) {
        lcd.print("**********");
      }
      break;

    case 7:

      lcd.setCursor(0, 0);
      lcd.print("NIGHT POWER");
      lcd.print(" ");
      PS = PWM_LW_MIN / 2.5;
      lcd.print(PS);
      lcd.print("%");
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        PWM_LW_MIN = PWM_LW_MIN + 25;

        delay(200);
        lcd.clear();
      }
      if (PWM_LW_MIN > 250)  {
        PWM_LW_MIN = 250;
      }
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        PWM_LW_MIN = PWM_LW_MIN - 25;

        delay(200);
        lcd.clear();
      }
      if (PWM_LW_MIN < 0)  {
        PWM_LW_MIN = 0;
      }
      lcd.setCursor(3, 1);
      if (PS >= 10 && PS < 20 ) {
        lcd.print("*");
      }
      if (PS >= 20 && PS < 30 ) {
        lcd.print("**");
      }
      if (PS >= 30 && PS < 40 ) {
        lcd.print("***");
      }
      if (PS >= 40 && PS < 50 ) {
        lcd.print("****");
      }
      if (PS >= 50 && PS < 60 ) {
        lcd.print("*****");
      }
      if (PS >= 60 && PS < 70 ) {
        lcd.print("******");
      }
      if (PS >= 70 && PS < 80 ) {
        lcd.print("*******");
      }
      if (PS >= 80 && PS < 90 ) {
        lcd.print("********");
      }
      if (PS >= 90 && PS < 100 ) {
        lcd.print("*********");
      }
      if (PS >= 100) {
        lcd.print("**********");
      }
      break;

    case 8:

      lcd.setCursor(1, 0);
      lcd.print("DISP DAY");
      lcd.print(" ");
      PS = ScD / 2.5;
      lcd.print(PS);
      lcd.print(" ");
      lcd.print("%");
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        ScD = ScD + 25;

        delay(200);
        lcd.clear();
      }
      if (ScD > 250)  {
        ScD = 250;

      }
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        ScD = ScD - 25;

        delay(200);
        lcd.clear();
      }
      if (ScD < 0)  {
        ScD = 0;
      }
      lcd.setCursor(3, 1);
      if (PS >= 10 && PS < 20 ) {
        lcd.print("*");
      }
      if (PS >= 20 && PS < 30 ) {
        lcd.print("**");
      }
      if (PS >= 30 && PS < 40 ) {
        lcd.print("***");
      }
      if (PS >= 40 && PS < 50 ) {
        lcd.print("****");
      }
      if (PS >= 50 && PS < 60 ) {
        lcd.print("*****");
      }
      if (PS >= 60 && PS < 70 ) {
        lcd.print("******");
      }
      if (PS >= 70 && PS < 80 ) {
        lcd.print("*******");
      }
      if (PS >= 80 && PS < 90 ) {
        lcd.print("********");
      }
      if (PS >= 90 && PS < 100 ) {
        lcd.print("*********");
      }
      if (PS >= 100) {
        lcd.print("**********");
      }
      break;

    case 9:

      lcd.setCursor(0, 0);
      lcd.print("DISP NIGHT");
      lcd.print(" ");
      PS = ScN / 2.5;
      lcd.print(PS);
      lcd.print(" ");
      lcd.print("%");
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        ScN = ScN + 25;

        delay(200);
        lcd.clear();
      }
      if (ScN > 250)  {
        ScN = 250;

      }
      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        ScN = ScN - 25;

        delay(150);
        lcd.clear();
      }
      if (ScN < 0)  {
        ScN = 0;
      }
      lcd.setCursor(3, 1);
      if (PS >= 10 && PS < 20 ) {
        lcd.print("*");
      }
      if (PS >= 20 && PS < 30 ) {
        lcd.print("**");
      }
      if (PS >= 30 && PS < 40 ) {
        lcd.print("***");
      }
      if (PS >= 40 && PS < 50 ) {
        lcd.print("****");
      }
      if (PS >= 50 && PS < 60 ) {
        lcd.print("*****");
      }
      if (PS >= 60 && PS < 70 ) {
        lcd.print("******");
      }
      if (PS >= 70 && PS < 80 ) {
        lcd.print("*******");
      }
      if (PS >= 80 && PS < 90 ) {
        lcd.print("********");
      }
      if (PS >= 90 && PS < 100 ) {
        lcd.print("*********");
      }
      if (PS >= 100) {
        lcd.print("**********");
      }
      break;

    case 10:

      lcd.setCursor(1, 0);
      lcd.print("SAVE TO MEMORY");
      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("SAVE GO");
        delay(500);
        for (int thisChar = 0; thisChar < 16; thisChar++) {
          lcd.setCursor(thisChar, 1);
          lcd.print("*");
          delay(50);
        }
        EEPROM.write(0, RH);
        EEPROM.write(1, RM);
        EEPROM.write(2, RD);
        EEPROM.write(3, ZH);
        EEPROM.write(4, ZM);
        EEPROM.write(5, ZD);
        lcd.clear();
        pos = 1;
      }
      break;

    case 11: // Часы
    case 12: // Минуты
    case 13: // День
    case 14: // Месяц
    case 15: // Год
      int ms = millis() % 500;
      lcd.setCursor(0, 0);
      lcd.print("DATE:");

      lcd.setCursor(6, 0);
      if (pos == 13 && (ms > 400)) {
        lcd.print("  ");
      } else {
        if (myTime.day() < 10) lcd.print("0");
        lcd.print(myTime.day(), DEC);
      }
      lcd.print("/");

      if (pos == 14 && (ms > 400)) {
        lcd.print("  ");
      } else {
        if (myTime.month() < 10) lcd.print("0");
        lcd.print(myTime.month(), DEC);
      }
      lcd.print("/");

      if (pos == 15 && (ms > 400)) {
        lcd.print("    ");
      } else {
        lcd.print(myTime.year(), DEC);
      }

      lcd.setCursor(0, 1);
      if (pos == 11 && (ms > 400)) {
        lcd.print("  ");
      } else {
        if (myTime.hour() < 10) lcd.print("0");
        lcd.print(myTime.hour(), DEC);
      }
      lcd.print(":");

      if (pos == 12 && (ms > 400)) {
        lcd.print("  ");
      } else {
        if (myTime.minute() < 10) lcd.print("0");
        lcd.print(myTime.minute(), DEC);
      }

      buttonState3 = digitalRead(buttonPin3);
      if (buttonState3 == HIGH) {
        int valueToSet = 0;
        switch (pos) {
          case 11:
            valueToSet = myTime.hour() + 1;
            if (valueToSet > 23) valueToSet = 0;
            RTC.adjust(DateTime(myTime.year(), myTime.month(), myTime.day(), valueToSet, myTime.minute(), 0));
            break;
          case 12:
            valueToSet = myTime.minute() + 1;
            if (valueToSet > 59) valueToSet = 0;
            RTC.adjust(DateTime(myTime.year(), myTime.month(), myTime.day(), myTime.hour(), valueToSet, 0));
            break;
          case 13:
            valueToSet = myTime.day() + 1;
            if (valueToSet > 31) valueToSet = 0;
            RTC.adjust(DateTime(myTime.year(), myTime.month(), valueToSet, myTime.hour(), myTime.minute(), 0));
            break;
          case 14:
            valueToSet = myTime.month() + 1;
            if (valueToSet > 12) valueToSet = 0;
            RTC.adjust(DateTime(myTime.year(), valueToSet, myTime.day(), myTime.hour(), myTime.minute(), 0));
            break;
          case 15:
            RTC.adjust(DateTime(valueToSet + 1, myTime.month(), myTime.day(), myTime.hour(), myTime.minute(), 0));
            break;
        }

        delay(200);
      }

      buttonState2 = digitalRead(buttonPin2);
      if (buttonState2 == HIGH) {
        int valueToSet = 0;
        switch (pos) {
          case 11:
            valueToSet = myTime.hour() - 1;
            if (valueToSet < 0) valueToSet = 23;
            RTC.adjust(DateTime(myTime.year(), myTime.month(), myTime.day(), valueToSet, myTime.minute(), 0));
            break;
          case 12:
            valueToSet = myTime.minute() - 1;
            if (valueToSet < 0) valueToSet = 59;
            RTC.adjust(DateTime(myTime.year(), myTime.month(), myTime.day(), myTime.hour(), valueToSet, 0));
            break;
          case 13:
            valueToSet = myTime.day() - 1;
            if (valueToSet < 0) valueToSet = 31;
            RTC.adjust(DateTime(myTime.year(), myTime.month(), valueToSet, myTime.hour(), myTime.minute(), 0));
            break;
          case 14:
            valueToSet = myTime.month() - 1;
            if (valueToSet < 0) valueToSet = 12;
            RTC.adjust(DateTime(myTime.year(), valueToSet, myTime.day(), myTime.hour(), myTime.minute(), 0));
            break;
          case 15:
            RTC.adjust(DateTime(myTime.year() - 1, myTime.month(), myTime.day(), myTime.hour(), myTime.minute(), 0));
            break;
        }

        delay(200);
      }
      break;
  }

  //delay (100);
  //wdt_reset();  // сброс сторожевого таймера
  //wdt_enable(WDTO_1S); // разрешение работы сторожевого таймера с тайм-аутом 1 с
}
