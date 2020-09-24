/*
    UVC sensor interface.
    Tar inn data fra 0-5V UVC sensor (GUVC-T21GH) og regner ut RMS.  Sender ut som 0-10V via DAC og Op.Amp.
    Og to digitalutganger for grenseverdier.

    2020-09-19 - V1.0
    Trond Mjåtveit Hansen
*/
#define DEBUG

#include <Wire.h>

#define uvcpin A0   //Analogpin UVC sensor er koplet på
#define pot1 A1     //Analogpin potentiometer 1 er koplet på
#define pot2 A2     //Analogpin potentiometer 2 er koplet på
#define limit0 2.0  //Grenseverdi mW/cm2 for utgang 0
#define limit1 5.0  //Grenseverdi mW/cm2 for utgang 1
#define qlimit0 2   //Arduino pinnenummer for utgang 0
#define qlimit1 3   //Arduino pinnenummer for utgang 1
#define DACaddr B1100000 //MCP4716A0T-E DAC i2c addresse

const double voltPerBit = 5.0 / 1023.0; //5 volt er referansespenningen
const double voltPermWcm = 0.71;      //UV sensorens volt per mW/cm2
const unsigned int sampleTime = 100;  //Antall millisekunder vi tar gjennomsnittet av.

unsigned long sensorRaw; //ubehandlet signal fra UVC-sensor
unsigned long sensorTotal;
unsigned int sensorNumSamples; //Variabel for faktisk antall avlesninger
unsigned int sensorRMS;
double voltRMS;
double mwattPerSqrCMtr; //Ferdig behandlet signal fra sensor i milliwatt/cm2

unsigned long lastTime;


void setup() {
#ifdef DEBUG
  Serial.begin(19200);
#endif
  pinMode(qlimit0, OUTPUT);
  pinMode(qlimit1, OUTPUT);
  Wire.begin();
}

void loop() {
  sensorTotal = 0;
  sensorNumSamples = 0;
  lastTime = millis();

  while (millis() - lastTime < sampleTime) {//Forsiktig med hvor lenge vi tar gjennomsnittet over.
    sensorRaw = analogRead(uvcpin);             //Det er bare plass til ca. 4100 maksavlesninger i en unsigned long.
                                            //500ms risikerer å gi overflow.
    sensorTotal = sensorTotal + (sensorRaw * sensorRaw);
    sensorNumSamples = sensorNumSamples + 1;
  }

#ifdef DEBUG
  Serial.print("Samples: ");
  Serial.println(sensorNumSamples);
#endif

  sensorRMS = sqrt(sensorTotal / sensorNumSamples);

#ifdef DEBUG
  Serial.print("RMS: ");
  Serial.println(sensorRMS);
#endif

  voltRMS = sensorRMS * voltPerBit;

#ifdef DEBUG
  Serial.print("Volt: ");
  Serial.println(voltRMS);
#endif

  mwattPerSqrCMtr = voltRMS / voltPermWcm;

#ifdef DEBUG
  Serial.print("Milliwatt per kvadratCM: ");
  Serial.println(mwattPerSqrCMtr);
#endif

  //Sjekk grenseverdier
  if (limit0 < mwattPerSqrCMtr) {
    digitalWrite(qlimit0, HIGH);
  } else {
    digitalWrite(qlimit0, LOW);
  }

  if (limit1 < mwattPerSqrCMtr) {
    digitalWrite(qlimit1, HIGH);
  } else {
    digitalWrite(qlimit1, LOW);
  }

  //Send data til DAC
  byte hibyte, lobyte;
  lobyte = sensorRMS << 2;
  hibyte = (sensorRMS >> 6) & 00111111;
  Wire.beginTransmission(DACaddr);
  Wire.write(hibyte);
  Wire.write(lobyte);
  Wire.endTransmission(true);

#ifdef DEBUG
  Serial.print("Highbyte: ");
  Serial.println(hibyte, BIN);
  Serial.print("Lowbyte: ");
  Serial.println(lobyte, BIN);
#endif

#ifdef DEBUG
  delay(50);
#endif
}
