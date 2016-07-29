// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <stdio.h>
#include <OneWire.h>
#include <Wire.h>
#include "RTClib.h"
#include <DallasTemperature.h>
#include <p018.h>
//#include "EmonLib.h"
//#include <SPI.h>
//#include <SD.h>

#define DS18B20 2
//#define SCT 1

//File myFile;
//EnergyMonitor emon1;
LCD disp(0x38);
OneWire oneWire(DS18B20);
RTC_DS1307 RTC;
DallasTemperature sensor(&oneWire);

int year, month, day, hour, minute;
int point_h_s[2], point_m_s[2], point_m_e[2], point_h_e[2];   //Horário de Ponta
int full_h_s = 11, full_m_s = 30, full_h_e = 17, full_m_e = 30;       //Horário de Cheia
int empty_h_s = 22, empty_h_e = 8;   //Horário de Vazio
int full_temp = 60;                             //Temperatura minima de Cheia
int empty_temp = 65;                            //Temperatura minima de vazio
int tensao = 230;

float temperature;
 
void setup () {
    Serial.begin(57600);
    Wire.begin();
    RTC.begin();
 
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

 sensor.begin();

 pinMode(8, OUTPUT);

// emon1.current(SCT, 11.299);

}

double GetTemp(void)  {
  unsigned int wADC;
  double t;

  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  //delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
}

float temp_sensor() {
  sensor.requestTemperatures();
  temperature = sensor.getTempCByIndex(0);

  return (temperature);
}

void lcd_po18(int type, float temperature, int relay, int hour, int minute)  {

  disp.lcd_cursor(0);
  disp.lcd_rowcol(0,0);
  disp.lcd_print("REG: ");
  disp.lcd_rowcol(1,0);
  disp.lcd_print("CIL: ");
  disp.lcd_rowcol(1,8);
  disp.lcd_print("CPU: ");
  disp.lcd_rowcol(0,5);
  
  if (type == 0)  {
    disp.lcd_print("VAZIO ");
  }
  if (type == 1)  {
    disp.lcd_print("CHEIO ");
  }
  if (type == 2)  {
    disp.lcd_print("PONTA ");
  }

  disp.lcd_rowcol(1,5);
  disp.lcd_print(temperature);
  disp.lcd_rowcol(1,13);
  disp.lcd_print(GetTemp());

  if (relay == 1) {
    disp.lcd_bl(10,0,0);
  }
  if (relay == 0) {
    disp.lcd_bl(10,10,10);
  }
  disp.lcd_rowcol(0,11);
  disp.lcd_print(hour);
  disp.lcd_print(":");

  if (minute < 10)  {
    disp.lcd_rowcol(0,14);
    disp.lcd_print("0");
    disp.lcd_print(minute);
  }
  else  {
    disp.lcd_print(minute);
  }
}

/*double Current()  {
  double Irms = emon1.calcIrms(1480);

  return (Irms);
}

double Power(double irms)  {
  double pw = irms*230;

  return (pw);
}

void SDCard(int month) {
  
  myFile = SD.open(month, FILE_WRITE);
}*/
 
void loop () {
  temperature = temp_sensor();
  
  DateTime now = RTC.now();
  year = now.year(), DEC;
  month = now.month(), DEC;
  day = now.day(), DEC;
  hour = now.hour(), DEC;
  minute = now.minute(), DEC;

  point_h_s[1] = 8;
  point_h_e[1] = 11;
  point_m_e[1] = 30;
  point_h_s[2] = 17;
  point_m_s[2] = 30;
  point_h_e[2] = 22;


  //Ponta
  if (hour >= point_h_s[1] && hour <= point_h_e[1] - 1) {
    digitalWrite(8, LOW);
    lcd_po18(2,temperature,0,hour,minute);
    delay(1000);
  }
  else if (hour == point_h_e[1] && minute <= point_m_e[1])  {
    digitalWrite(8, LOW);
    lcd_po18(2,temperature,0,hour,minute);
    delay(1000);
  }
  //Cheia
  else if (hour >= full_h_s + 1 && hour <= full_h_e - 1)  {
    if (temperature <= full_temp) {
      while(temperature <= full_temp + 5)  {
        hour = now.hour(), DEC;
        minute = now.minute(), DEC;
        temperature = temp_sensor();
        digitalWrite(8, HIGH);
        lcd_po18(1,temperature,1,(now.hour(), DEC),(now.minute(), DEC));
        delay(1000);
      }
    }
    else  {
      digitalWrite(8, LOW);
      lcd_po18(1,temperature,0,hour,minute);
      delay(1000);
    }
  }
  else if (hour == full_h_s && minute > full_m_s)  {
    if (temperature <= full_temp) {
      while(temperature <= full_temp + 5)  {
        hour = now.hour(), DEC;
        minute = now.minute(), DEC;
        temperature = temp_sensor();
        lcd_po18(1,temperature,1,hour,minute);
        digitalWrite(8, HIGH);
        delay(1000);
      }
    }
    else  {
      digitalWrite(8, LOW);
      lcd_po18(1,temperature,0,hour,minute);
      delay(1000);
    }
  }

  else if (hour == full_h_e && minute < full_m_e) {
    if (temperature <= full_temp) {
      while(temperature <= full_temp + 5)  {
        hour = now.hour(), DEC;
        minute = now.minute(), DEC;
        temperature = temp_sensor();
        lcd_po18(1,temperature,1,hour,minute);
        digitalWrite(8, HIGH);
        delay(1000);
      }
    }
    else  {
      digitalWrite(8, LOW);
      lcd_po18(1,temperature,0,hour,minute);
      delay(1000);
    }
  }
  //Ponta
  else if (hour >= point_h_s[2] + 1 && hour < point_h_e[2]) {
    digitalWrite(8, LOW);
    lcd_po18(2,temperature,0,hour,minute);
    delay(1000);
  }

  else if (hour == point_h_s[2] && minute >= point_m_s[2])  {
    digitalWrite(8, LOW);
    lcd_po18(2,temperature,0,hour,minute);
    delay(1000);
  }
  //Vazio
  else if (hour >= empty_h_s && hour <= 23) {
    if (temperature <= empty_temp)  {
      while(temperature <= empty_temp + 10) {
        hour = now.hour(), DEC;
        minute = now.minute(), DEC;
        temperature = temp_sensor();
        lcd_po18(0,temperature,1,hour,minute);
        delay(1000);
      }
    }
    else  {
      digitalWrite(8, LOW);
      lcd_po18(0,temperature,0,hour,minute);
      delay(1000);
    }
  }
  else if (hour >= 0 && hour < empty_h_e) {
    if (temperature <= empty_temp)  {
      while(temperature <= empty_temp + 10) {
        hour = now.hour(), DEC;
        minute = now.minute(), DEC;
        temperature = temp_sensor();
        lcd_po18(0,temperature,1,hour,minute);
        digitalWrite(8, HIGH);
        delay(1000);
      }
    }
    else  {
      digitalWrite(8, LOW);
      lcd_po18(0,temperature,0,hour,minute);
      delay(1000);
    }
  }
}
