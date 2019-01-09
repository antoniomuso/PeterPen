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
const int TH_FORCE = 100;
// Delay del loop principale
const int LOOP_DELAY = 10;
// Tempo di attesa prima di poter riscrivere un altra parola
const int TIME_WAIT = 2000;
// Tempo di scrittura massima di una parola
const int MAX_TIME = 10000;
// Dimensione massima dell array dei dati
const int ARRAY_DIM = MAX_TIME / LOOP_DELAY;
const bool DEBUG = true;

const char RAWDATA_JSON_PROTOTYPE[] = "{\"AcX\": %"PRId16",\"AcY\": %"PRId16",\"AcZ\": %"PRId16",\"Pres\": %"PRId16",\"GyX\": %"PRId16",\"GyY\": %"PRId16",\"GyZ\": %"PRId16"}"; 

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

bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr, bool Debug);
void setMPU6050scales(byte addr, uint8_t Gyro, uint8_t Accl);
void getMPU6050scales(byte addr, uint8_t &Gyro, uint8_t &Accl);
void blink();
//scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug);

// variable
int16_t pressionSensor;
rawdata next_sample;
bool isOpen = true;
int counter_led = 0;
int counter_writing = 0;
PEN_STATUS status_pen = READY;
struct rawdata arr[ARRAY_DIM];

// Dati di connessione
WiFiClient client;
const char* host = "192.168.43.242";
bool isConnected = false;

void connect () {
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
  if (client.connect(host, 8080))
  {
    isConnected = true;
  }
  else
  {
    isConnected = false;
  }
}

void setup()
{

  Wire.begin();
  Serial.begin(9600);
  pinMode(0, OUTPUT);
  

  mpu6050Begin(MPU_addr);
  //connect();
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
    status = READY;
  }
  else if (status == WRITING)
  {
    digitalWrite(0, LOW);
    status = WRITING;
  }
  else
  {
    status = ERROR;
    blink();
    status = READY;
  }
}

void sendData()
{

  // vanno inviati counter_writing e LOOP_DELAY
  for (int i = 0; i < 10; i++)
  {

  }
};

void readData(byte addr)
{

  // This function reads the raw 16-bit data values from
  // the MPU-6050

  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 14, true); // request a total of 14 registers
  rawdata data;

  arr[counter_writing].AcX = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].AcY = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].AcZ = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].pressure = (pressionSensor);
  arr[counter_writing].GyX = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].GyY = (Wire.read() << 8 | Wire.read());
  arr[counter_writing].GyZ = (Wire.read() << 8 | Wire.read());

  if (DEBUG && counter_writing%200 == 0)
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
  else
  {
    setMPU6050scales(MPU_addr, 0b00000000, 0b00010000);
    readData(MPU_addr);
    counter_led++;
    counter_writing++;

    if (counter_led == TIME_WAIT / LOOP_DELAY || (counter_writing * LOOP_DELAY) >= MAX_TIME)
    {
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
  Wire.requestFrom(addr, 2, true); // request a total of 14 registers
  Gyro = (Wire.read() & (bit(3) | bit(4))) >> 3;
  Accl = (Wire.read() & (bit(3) | bit(4))) >> 3;
}
