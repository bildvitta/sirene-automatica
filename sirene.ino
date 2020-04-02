/*
 * Temporizador para sirenes (pt_BR)
 * 
 * Copyright 2020 Bild Vitta
 * Hugo Demiglio
 *
 * Testado com Arduino UNO (IDE 1.8.8)
 * Verificar abaixo as dependencias de bibliotecas
 */

// Pins

#define key1 11
#define key2 10
#define key3 13
#define key4 12

#define relay 7

// EEPROM
#include <EEPROM.h>

// RTC
#include <DS1307.h>
DS1307 rtc(A2, A3);

// Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

// Keypad
int key_status, last_key = 0, key1_status, key2_status, key3_status, key4_status;

// System Status
int screen = 0, mode = 0, tone_mode = 0, tone_duration = 0, tone_interval = 6, start_hour = 7, end_hour = 18;

void setup() {
  // RTC
  rtc.halt(false);
   
//  rtc.setDOW(WEDNESDAY);
//  rtc.setTime(18, 47, 0);
//  rtc.setDate(1, 4, 2020);
  
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);

  // Keypad
  pinMode(key1, INPUT_PULLUP);
  pinMode(key2, INPUT_PULLUP);
  pinMode(key3, INPUT_PULLUP);
  pinMode(key4, INPUT_PULLUP);

  // Relays
  pinMode(relay, OUTPUT);

  // Display
  lcd.begin (16, 2);
  lcd.clear();
  lcd.setBacklight(HIGH);

  // Serial 
  Serial.begin(9600);

  // EEPROM
  tone_mode = EEPROM.read(0);
  tone_duration = EEPROM.read(1);
  tone_interval = EEPROM.read(2);
  start_hour = EEPROM.read(3);
  end_hour = EEPROM.read(4);

  if(start_hour > 23 || start_hour < 0) start_hour = 7;
  if(end_hour > 23 || end_hour < 0) end_hour = 18;
  if(tone_duration > 30 || end_hour < 0) tone_duration = 5;

  init_system();
}

void init_system() {
  digitalWrite(relay, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("   Bild Vitta"); 
  lcd.setCursor(0, 1);
  lcd.print("  Temporizador");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("  Versao 1.0.0");
  delay(2000);

  lcd.clear();
}

void loop() {
  get_inputs();

  // cron();

  switch (screen) {
    case 0:
    standby_screen();
    break;
    case 1:
    incrementer_screen("  Hora inicial  ", start_hour);
    break;
    case 2:
    incrementer_screen("   Hora final   ", end_hour);
    break;
    case 3:
    incrementer_label_screen("Duracao do toque", "seg", tone_duration);
    break;
  }
}

void standby_screen() {
  lcd.setCursor(0, 0);
  lcd.print("   Bild Vitta   "); 
  lcd.setCursor(1, 1);
  lcd.print(rtc.getDateStr(FORMAT_SHORT));

  lcd.setCursor(6, 1);
  lcd.print(" ");
  lcd.print(rtc.getTimeStr());

  // delay (1000);
}

void incrementer_screen(const String& text, int data) {
  lcd.setCursor(0, 0);
  lcd.print(text);
  lcd.setCursor(0, 1);
  lcd.print("   +   ");
  if(data < 10) lcd.print("0");
  lcd.print(data);
  lcd.print("   -  ");
}

void incrementer_label_screen(const String& text, const String& label, int data) {
  lcd.setCursor(0, 0);
  lcd.print(text);
  lcd.setCursor(0, 1);
  lcd.print("   +   ");
  if(data < 10) lcd.print(" ");
  lcd.print(data);
  lcd.print(" ");
  lcd.print(label);
  lcd.print("  -  ");
}

void get_inputs(){
  key1_status = digitalRead(key1);
  key2_status = digitalRead(key2);
  key3_status = digitalRead(key3);
  key4_status = digitalRead(key4);

  if(!key1_status) {
    key_status = 1;
  } else if(!key2_status) {
    key_status = 2;
  } else if(!key3_status) {
    key_status = 3;
  } else if(!key4_status) {
    key_status = 4;
  } else {
    key_status = 0;
  }

  if(key_status == 1 && last_key != 1) {
    screen++;
    lcd.clear();
    if(screen > 3) screen = 0;
  }

  if(key_status != last_key){
    switch (screen) {
      case 1:
        if(key_status == 2) start_hour += 1;
        if(key_status == 3) start_hour -= 1;
        if(start_hour < 0) start_hour = 0;
        if(start_hour > end_hour) start_hour = end_hour;

        EEPROM.write(3, start_hour);
      break;
      case 2:
        if(key_status == 2) end_hour += 1;
        if(key_status == 3) end_hour -= 1;
        if(end_hour < start_hour) end_hour = start_hour;
        if(end_hour > 23) end_hour = 23;

        EEPROM.write(4, end_hour);
      break;
      case 3:
        if(key_status == 2) tone_duration += 1;
        if(key_status == 3) tone_duration -= 1;
        if(tone_duration < 1) tone_duration = 1;
        if(tone_duration > 30) tone_duration = 30;

        EEPROM.write(1, tone_duration);
      break;
    }
  }

  if(key_status == 4){
    // play_tone();

    lcd.setCursor(0,0);
    lcd.print("   Acionamento  ");
    lcd.setCursor(0,1);
    lcd.print("     Manual     ");
    delay(2000);
    lcd.clear();
  }

  last_key = key_status;
}