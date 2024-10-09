
/*************************************************************
  Download latest ERa library here:
    https://github.com/eoh-jsc/era-lib/releases/latest
    https://www.arduino.cc/reference/en/libraries/era
    https://registry.platformio.org/libraries/eoh-ltd/ERa/installation

    ERa website:                https://e-ra.io
    ERa blog:                   https://iotasia.org
    ERa forum:                  https://forum.eoh.io
    Follow us:                  https://www.fb.com/EoHPlatform
 *************************************************************/

// Enable debug console
// Set CORE_DEBUG_LEVEL = 3 first
// #define ERA_DEBUG

#define DEFAULT_MQTT_HOST "mqtt1.eoh.io"

// You should get Auth Token in the ERa App or ERa Dashboard
#define ERA_AUTH_TOKEN "cda9708a-9ff1-4280-9ac1-2876a7381bae"

#include <Arduino.h>
#include <ERa.hpp>
#include <ERa/ERaTimer.hpp>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>

const char ssid[] = "Virus Infected Wifi";
const char pass[] = "29065035762848";

Adafruit_MPU6050 mpu;
Adafruit_BMP085  bmp;

int Set = 0;
int Value[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
double a=0, b=0, x=0, y=0;
uint64_t tim = 0;
float volMax = 2.0, volMin = 0.4, volConvCons = 0.004882814, volSen = 0;
float windSpeedMin = 0;
float windSpeedMax = 30;
int WindSpeed = 0, prevWindSpeed = 0;
int Direction = 0, direc = 0;
int Mua = 0;
float temp = 0, pres = 0, alti = 0, accX = 0, accY = 0, accZ = 0, preAccX = 0, preAccY = 0, preAccZ = 0;

ERaTimer timer;

/* This function print uptime every second */
void timerEvent() {
    ERa.virtualWrite(V10, (ERaMillis() / 1000L / 3600L));
    ERa.virtualWrite(V11, ((ERaMillis() / 1000L / 60L) - int(ERaMillis() / 1000L / 3600L) * 60L));
    ERa.virtualWrite(V12, (ERaMillis() / 1000L) % 60L);
    ERa.virtualWrite(V0 , WindSpeed);
    ERa.virtualWrite(V1 , direc    );
    ERa.virtualWrite(V2 , accX     );
    ERa.virtualWrite(V3 , accY     );
    ERa.virtualWrite(V4 , accZ     );
    ERa.virtualWrite(V5 , pres     );
    ERa.virtualWrite(V6 , temp     );
    ERa.virtualWrite(V7 , alti     );
    ERa.virtualWrite(V8 , Mua      );
    ERa.virtualWrite(V9 , Set      );
    ERA_LOG("Timer", "Uptime: %d", ERaMillis() / 1000L);
}

void setup() {
    /* Setup debug console */
    Serial.begin(9600);
    Wire.begin();

    ERa.begin(ssid, pass);
    timer.setInterval(1000L, timerEvent);

  while (!Serial)
  delay(10); // will pause Zero, Leonardo, etc until serial console opens
  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);	// Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
  Serial.println("");
  delay(100);

}

void loop() {
  TinHieuSet();
  if (millis() - tim > 60000) {
    Set = 0;
    tim = millis();
  }
  DoApSuatVaGiaToc();
  DoHuongGio();
  DoTocDoGio();
  Mua = DoLuongMua();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;

    ERa.run();
    timer.run();
}

void DoApSuatVaGiaToc() {
    temp = bmp.readTemperature();
    pres = bmp.readPressure()   ;
    alti = bmp.readAltitude()   ;
}

void DoHuongGio() {
  int sensorValue = analogRead(A3);
  float voltage = sensorValue * 3.3 / 4095.0;
  Direction = map(sensorValue, 0, 4095, 0, 360);
  if (Direction < 40) direc = 0;
  else if (Direction >= 40 && Direction < 90) direc = 45;
  else if (Direction >= 90 && Direction < 150) direc = 90;
  else if (Direction >= 150 && Direction < 210) direc = 135;
  else if (Direction >= 210 && Direction < 260) direc = 180;
  else if (Direction >= 260 && Direction < 300) direc = 225;
  else if (Direction >= 300 && Direction < 330) direc = 270;
  else if (Direction >= 330 && Direction < 360) direc = 315;
  delay(1);  // delay in between reads for stability
}

void DoTocDoGio() {
  int sensorValue = analogRead(A6);
  float voltage = sensorValue * 3.3 / 4095.0;
  volSen = sensorValue * volConvCons / 4.0;

  if (volSen <= volMin) WindSpeed = 0;
  else WindSpeed = ((volSen - volMin) * windSpeedMax / (volMax - volMin)) * 2.232694;
  x = WindSpeed;
  if (x >= y) y = x; else y = y;
  a = volSen;
  if (a >= b) b = a; else b = b;
}

void TinHieuSet() {
  int a = 0;
  for(int i = 0; i < 10; i++){
    Value[i] = analogRead(A7);
    if(Value[i] > 2000) a++;
    delay(2);
  }
  if(a >= 9) {
    Set++;
    Serial.println(Set);
  }
}

int DoLuongMua() {
  int sensorValue = analogRead(A0);
  int Nguong[4] = {1000, 2000, 3000 ,4000};
  if (sensorValue < Nguong[0]) return 0;
  else if ((sensorValue >= Nguong[0]) && (sensorValue < Nguong[1])) return 25;
  else if ((sensorValue >= Nguong[1]) && (sensorValue < Nguong[2])) return 50;
  else if ((sensorValue >= Nguong[2]) && (sensorValue < Nguong[3])) return 75;
  else if (sensorValue >= Nguong[3])  return 100;
}