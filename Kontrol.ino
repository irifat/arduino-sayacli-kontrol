#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ButtonKing.h>
//optoisolator karti ters mantik calisiyor. Kullanim kolayligi icin ters degisken kullaniyorum
#define ON LOW
#define OFF HIGH
int ekranTazele = 0;
int basamakSayisi = 0;
int kontrol = 0;
int sayacSifirla = 0;
unsigned long kronometreBasla = 0; //zamana bagli islemlerde gecen zaman farkini hesaplamak icin kullanilacak
unsigned long kronometreSimdi = 0;
const int sensorPin = 8;
const int surucu = 10;
const int lamba = 9;
const int encoderPin = 6;
ButtonKing dEncoder(encoderPin, true);
ButtonKing dSensor(sensorPin, true);
//ButtonKing dBaslat(baslatPin, true);
//Ekran cismini olustur
LiquidCrystal_I2C lcd(0x27, 16, 2);
long hedef = 1000;
long sayac = 0;
char durum = 'A';  //sistemin calisip calismadigini gosterir. A= beklemede, B= calisiyor C= sayac sifirla D= sayac hedefe ulasti ve gorev bitti

//Encoder icin kullanilacak degiskenler
#define outputA 4
#define outputB 5
int buttonState = 0;
int aState;
int aLastState;


void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  //Dugme fonksiyonlari (her dugme icin ayri olacak)
  dEncoder.setClick(encoderClick);
  dEncoder.setDoubleClick(encoderDoubleClick);
  dEncoder.setLongClickStart(encoderLongPressStart);
  dEncoder.setLongClickStop(encoderLongPressStop);
  dSensor.setClick(sensorClick);
  dSensor.setLongClickStop(sensorLongPressStop);

  pinMode (outputA, INPUT);
  pinMode (outputB, INPUT);
  pinMode (sensorPin, INPUT_PULLUP);
  pinMode (surucu, OUTPUT);
  pinMode (lamba, OUTPUT);
  aLastState = digitalRead(outputA);
  //cikis pinlerini kapat
  digitalWrite(surucu, LOW);
  digitalWrite(lamba, LOW);
  //ekrani hazirla
  ekranaYaz(durum, 0);
  ekranaYaz(durum, 1);
}

void loop() {
  dEncoder.isClick();
  dSensor.isClick();
  encoderOku();
  switch (durum) {
    case 'A':   //bekleme durumu
      digitalWrite(surucu, LOW);
      digitalWrite(lamba, LOW);
      break;
    case 'B':   //calisma durumu
      if (sayac >= hedef) {
        durum = 'D';  //is bitti
        digitalWrite(surucu, LOW);
        digitalWrite(lamba, LOW);
        Serial.print("loop durum: ");
        Serial.println(durum);
        ekranaYaz(durum, 0);
      } else {
        digitalWrite(surucu, HIGH);
        digitalWrite(lamba, HIGH);
      }
      break;
    case 'C':   //sayaci sifirla
      if(sayacSifirla == 0) {
        kronometreBasla = millis();
        sayacSifirla = 1;
      }
      kronometreSimdi = millis();
      if ((kronometreSimdi - kronometreBasla)>=3000) { //zaman asimi gerceklesti, sayac sifirlama islemi iptal
        durum = 'A';
        sayacSifirla = 0;
        ekranTazele = 0;
      }
      ekranaYaz(durum, 0);
      ekranaYaz(durum, 1);
      break;
  }
}

void encoderOku() {
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState) {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState) {
      if (buttonState == 1) {
        kontrol = hedef + 10;
        kacBasamakli(kontrol);
        if (basamakSayisi < 5) {
          hedef = hedef + 10;
        }
      } else {
        kontrol = hedef + 100;
        kacBasamakli(kontrol);
        if (basamakSayisi < 5) {
          hedef = hedef + 100;
        }
      }
    } else {
      if (buttonState == 1 ) {
        if ((hedef - 10) > 0) {
          hedef = hedef - 10;
        }
      } else {
        if ((hedef - 100) > 0) {
          hedef = hedef - 100;
        }
      }
    }
    ekranTazele = 0;
    Serial.print("encoder oku - durum:");
    Serial.print(durum);
    Serial.print(" - ekran tazele :");
    Serial.println(ekranTazele);
    ekranaYaz(durum, 0);
  }
  aLastState = aState; // Updates the previous state of the outputA with the current state
}
void kacBasamakli(int girdi) {
  int basamak = 1;
  int bolum = girdi / 10;
  while (bolum > 0) {
    bolum = bolum / 10;
    basamak ++;
  }
  basamakSayisi = basamak;
}
void ekranaYaz(char ekranTipi, int satirNo) {
  if (ekranTazele == 0) {
    switch (ekranTipi) {
      case 'A':
      case 'B':
        switch (satirNo) {
          case 0:
            char satir1[17];
            sprintf(satir1, "Hedef: %4d     ", hedef);
            lcd.home();
            lcd.print(satir1);
            Serial.print("Default-0 - ");
            Serial.println(ekranTazele);
            break;
          case 1:
            char satir2[17];
            sprintf(satir2, "Mevcut: %4d    ", sayac);
            lcd.setCursor(0, 1);
            lcd.print(satir2);
            Serial.print("default-1 - ");
            Serial.println(ekranTazele);
            break;
        }
        break;
      case 'C': //sayac sifirlama
        switch (satirNo) {
          case 0:
            char satir1[17];
            sprintf(satir1, "Sayaci sifirla  ");
            lcd.home();
            lcd.print(satir1);
            Serial.print("C-0 - ");
            Serial.println(ekranTazele);
            break;
          case 1:
            char satir2[17];
            sprintf(satir2, "Cift bas = evet ");
            lcd.setCursor(0, 1);
            lcd.print(satir2);
            ekranTazele = 1;
            Serial.print("C-1 - ");
            Serial.println(ekranTazele);
            break;
        }
        break;
      case 'D': //is bitti
        switch (satirNo) {
          case 0:
            char satir1[17];
            sprintf(satir1, "Hedefe ulasildi");
            lcd.home();
            lcd.print(satir1);
            ekranTazele = 1;
            Serial.print("D-0 - ");
            Serial.println(ekranTazele);
            break;
        }
    }
  }
}

void encoderClick() {
  Serial.print( "encoder click - durum:");
  Serial.print(durum);
  switch (durum) {
    case 'A':
    case 'B':
      ekranTazele = 0;
      if (buttonState == 1) {
        buttonState = 0;
      } else {
        buttonState = 1;
      }
      break;
    case 'C':
      break;
    case 'D':
      ekranTazele = 0;
      durum = 'A';
      sayac = 0;
      ekranaYaz(durum, 0);
      ekranaYaz(durum, 1);
      break;
  }

}

void encoderLongPressStart() {
  if (durum == 'A') {
    if (ekranTazele == 0) {
      lcd.clear();
      lcd.home();
      lcd.print(F("Baslat "));
    }
    ekranTazele = 1;
  } else {
    if (ekranTazele == 0) {
      lcd.clear();
      lcd.home();
      lcd.print(F("Durdur "));
    }
    ekranTazele = 1;
  }
}

void encoderLongPressStop() {
  if (durum == 'A') {
    durum = 'B';
    ekranTazele = 0;
    Serial.print("longPress Start - Durum: ");
    Serial.println(durum);
    ekranaYaz(durum, 0);
    ekranaYaz(durum, 1);
  } else {
    durum = 'A';
    ekranTazele = 0;
    ekranaYaz(durum, 0);
    ekranaYaz(durum, 1);
  }
}

void encoderDoubleClick() {
  if (durum == 'A' && sayacSifirla == 0) {
    durum = 'C';
  }
  if (durum == 'C' && sayacSifirla == 1) {
    sayac = 0;
    sayacSifirla = 0;
    ekranTazele = 0;
    durum = 'A';
    ekranaYaz(durum,0);
    ekranaYaz(durum,1);
    
  }
}

void sensorClick() {
  if (durum == 'B') {
    sayac++;
    Serial.print("sensor click ");
    Serial.println(sayac);
    ekranaYaz(durum, 1);
  }
}
void sensorLongPressStop() {
  if (durum == 'B') {
    sayac++;
    Serial.print("sensor long click ");
    Serial.println(sayac);
    ekranaYaz(durum, 1);
  }
}
