#include "Wire.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>

const uint8_t MPU_addr = 0x68; // I2C address of the MPU-6050

const float MPU_GYRO_250_SCALE = 131.0;
const float MPU_GYRO_500_SCALE = 65.5;
const float MPU_GYRO_1000_SCALE = 32.8;
const float MPU_GYRO_2000_SCALE = 16.4;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0;
const float MPU_ACCL_16_SCALE = 2048.0;

// Threshold del sensore di pressione per rilevare la scrittura
const int TH_FORCE = 200;
// Delay del loop principale
const int LOOP_DELAY = 10;
// Tempo di attesa prima di poter riscrivere un altra parola
const int _TIME_WAIT = 2000;
// Tempo di scrittura massima di una parola
const int MAX_TIME = 10000;
// Dimensione massima dell array dei dati
const int ARRAY_DIM = MAX_TIME / LOOP_DELAY;
const bool DEBUG = false;

const char RAWDATA_JSON_PROTOTYPE[] = "[%f,%f,%f,%.0f,%f,%f,%f]";

// Enum per lo status della penna
enum PEN_STATUS
{
  WRITING, // La penna sta scrivendo il led è spento
  READY,   // La penna è pronta per scrivere una parola nuova il led è acceso
  ERROR    // l'utente ha tenuto la penna in scrittura per più di MAX_TIME il led lampeggia
};

struct rawdata
{
  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;
  int16_t pressure;
  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;
};

struct scaleddata
{
  float AcX;
  float AcY;
  float AcZ;
  float pressure;
  float GyX;
  float GyY;
  float GyZ;
};

bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr);
void setMPU6050scales(byte addr, uint8_t Gyro, uint8_t Accl);
void getMPU6050scales(byte addr, uint8_t &Gyro, uint8_t &Accl);
scaleddata convertRawToScaled(byte addr, rawdata value);
void blink();
//scaleddata convertRawToScaled(byte addr, rawdata value, bool Debug);

// variable
int16_t pressionSensor;
bool isOpen = true;
int counter_led = 0;
int counter_writing = 0;
PEN_STATUS status_pen = READY;
struct rawdata arr[ARRAY_DIM];

// Dati di connessione
WiFiClient client;
const char *host = "192.168.43.242";
bool isConnected = false;

void connect()
{
  WiFi.begin("HUAWEI P20", "1234abcd");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    blink();
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  while (!client.connect(host, 8080))
  {
    blink();
  }
}

void setup()
{

  Wire.begin();
  Serial.begin(9600);
  pinMode(0, OUTPUT);

  mpu6050Begin(MPU_addr);
  connect();
}

// Effettua il lampeggiamento del led
void blink()
{
  bool stat_led = true;
  for (int i = 0; i < 20; i++)
  {
    if (stat_led)
    {
      stat_led = false;
      digitalWrite(0, LOW);
    }
    else
    {
      stat_led = true;
      digitalWrite(0, HIGH);
    }
    delay(200);
  }
}

void setStatus(PEN_STATUS status)
{
  if (status == READY)
  {
    digitalWrite(0, HIGH);
    status_pen = READY;
  }
  else if (status == WRITING)
  {
    digitalWrite(0, LOW);
    status_pen = WRITING;
  }
  else
  {
    status_pen = ERROR;
    blink();
    status_pen = READY;
  }
}

void sendData()
{
  Serial.println("Sending data...");
  // vanno inviati counter_writing e LOOP_DELAY
  client.print("[");
  for (int i = 0; i < counter_writing; i++)
  {
    scaleddata data = convertRawToScaled(MPU_addr, arr[i]);
    client.printf(RAWDATA_JSON_PROTOTYPE, data.AcX, data.AcY, data.AcZ, data.pressure, data.GyX, data.GyY, data.GyZ);
    if (i != counter_writing - 1)
      client.print(",");
  }
  client.println("]");
};

void readData(byte addr)
{

  // This function reads the raw 16-bit data value from
  // the MPU-6050

  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, (size_t)14, true); // request a total of 14 registers
  rawdata data;

  arr[counter_writing].AcX = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].AcY = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].AcZ = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].pressure = (pressionSensor);
  arr[counter_writing].GyX = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].GyY = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].GyZ = (Wire.read() << 8 | Wire.read());

  if (DEBUG && counter_writing % 200 == 0)
  {

    Serial.println(arr[counter_writing].AcX); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    Serial.println(arr[counter_writing].AcY); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    Serial.println(arr[counter_writing].AcZ); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Serial.print("Pression: ");
    Serial.println(arr[counter_writing].pressure);
    Serial.println(arr[counter_writing].GyX); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    Serial.println(arr[counter_writing].GyY); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    Serial.println(arr[counter_writing].GyZ); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    //arr[counter_writing].pressure = pressionSensor;
    Serial.print("Counter writer: ");
    Serial.println(counter_writing);
    Serial.flush();
  }
}

void loop()
{
  // Read data of pression sensor from pin A0
  pressionSensor = analogRead(0);
  // Accelerometer

  if (pressionSensor > TH_FORCE && (counter_writing * LOOP_DELAY) < MAX_TIME)
  {
    setStatus(WRITING);
    setMPU6050scales(MPU_addr, 0b00000000, 0b00010000);
    readData(MPU_addr);

    counter_led = 0;
    counter_writing++;
  }
  else if (pressionSensor > TH_FORCE)
  {
    setStatus(ERROR);

    counter_led = 0;
    counter_writing = 0;
  }
  else if (status_pen != READY)
  {
    //Serial.println("Dentro elseif");
    setMPU6050scales(MPU_addr, 0b00000000, 0b00010000);
    readData(MPU_addr);
    counter_led++;
    counter_writing++;

    if (counter_led == _TIME_WAIT / LOOP_DELAY || (counter_writing * LOOP_DELAY) >= MAX_TIME)
    {
      //Serial.print("Counter Led: ");
      //Serial.println(counter_led);
      //Serial.print("Counter writing: ");
      //Serial.println(counter_writing);

      counter_writing -= counter_led;
      sendData();
      setStatus(READY);

      counter_writing = 0;
      counter_led = 0;
    }
  }

  //Serial.print(pressionSensor);
  //Serial.print("\n");
  delay(LOOP_DELAY); // Wait 5 seconds and scan again
}

void mpu6050Begin(byte addr)
{
  // This function initializes the MPU-6050 IMU Sensor
  // It verifys the address is correct and wakes up the
  // MPU.
  if (checkI2c(addr))
  {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    delay(30); // Ensure gyro has enough time to power up
  }
}

bool checkI2c(byte addr)
{
  // We are using the return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.
  Serial.println(" ");
  Wire.beginTransmission(addr);

  if (Wire.endTransmission() == 0)
  {
    Serial.print(" Device Found at 0x");
    Serial.println(addr, HEX);
    return true;
  }
  else
  {
    Serial.print(" No Device Found at 0x");
    Serial.println(addr, HEX);
    return false;
  }
}

void setMPU6050scales(byte addr, uint8_t Gyro, uint8_t Accl)
{
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // write to register starting at 0x1B
  Wire.write(Gyro); // Self Tests Off and set Gyro FS to 250
  Wire.write(Accl); // Self Tests Off and set Accl FS to 8g
  Wire.endTransmission(true);
}

void getMPU6050scales(byte addr, uint8_t &Gyro, uint8_t &Accl)
{
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, (size_t)2, true); // request a total of 14 registers
  Gyro = (Wire.read() & (bit(3) | bit(4))) >> 3;
  Accl = (Wire.read() & (bit(3) | bit(4))) >> 3;
}

scaleddata convertRawToScaled(byte addr, rawdata value)
{

  scaleddata data;
  float scale_value = 0.0;
  byte Gyro, Accl;

  getMPU6050scales(MPU_addr, Gyro, Accl);

  if (DEBUG)
  {
    Serial.print("Gyro Full-Scale = ");
  }

  switch (Gyro)
  {
  case 0:
    scale_value = MPU_GYRO_250_SCALE;
    if (DEBUG)
    {
      Serial.println("±250 °/s");
    }
    break;
  case 1:
    scale_value = MPU_GYRO_500_SCALE;
    if (DEBUG)
    {
      Serial.println("±500 °/s");
    }
    break;
  case 2:
    scale_value = MPU_GYRO_1000_SCALE;
    if (DEBUG)
    {
      Serial.println("±1000 °/s");
    }
    break;
  case 3:
    scale_value = MPU_GYRO_2000_SCALE;
    if (DEBUG)
    {
      Serial.println("±2000 °/s");
    }
    break;
  default:
    break;
  }

  data.GyX = (float)value.GyX / scale_value;
  data.GyY = (float)value.GyY / scale_value;
  data.GyZ = (float)value.GyZ / scale_value;
  data.pressure = (float) value.pressure;

  scale_value = 0.0;

  if (DEBUG)
  {
    Serial.print("Accl Full-Scale = ");
  }
  switch (Accl)
  {
  case 0:
    scale_value = MPU_ACCL_2_SCALE;
    if (DEBUG)
    {
      Serial.println("±2 g");
    }
    break;
  case 1:
    scale_value = MPU_ACCL_4_SCALE;
    if (DEBUG)
    {
      Serial.println("±4 g");
    }
    break;
  case 2:
    scale_value = MPU_ACCL_8_SCALE;
    if (DEBUG)
    {
      Serial.println("±8 g");
    }
    break;
  case 3:
    scale_value = MPU_ACCL_16_SCALE;
    if (DEBUG)
    {
      Serial.println("±16 g");
    }
    break;
  default:
    break;
  }
  data.AcX = (float)value.AcX / scale_value;
  data.AcY = (float)value.AcY / scale_value;
  data.AcZ = (float)value.AcZ / scale_value;

  //value.Tmp = (float)value.Tmp / 340.0 + 36.53;

  if (DEBUG)
  {
    Serial.print(" GyX = ");
    Serial.print(value.GyX);
    Serial.print(" °/s| GyY = ");
    Serial.print(value.GyY);
    Serial.print(" °/s| GyZ = ");
    Serial.print(value.GyZ);
    //Serial.print(" °/s| Tmp = ");
    //Serial.print(value.Tmp);
    Serial.print(" °C| AcX = ");
    Serial.print(value.AcX);
    Serial.print(" g| AcY = ");
    Serial.print(value.AcY);
    Serial.print(" g| AcZ = ");
    Serial.print(value.AcZ);
    Serial.println(" g");
  }
  return data;
}