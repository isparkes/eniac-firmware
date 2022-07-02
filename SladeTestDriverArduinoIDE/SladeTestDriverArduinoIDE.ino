#include <Wire.h>
#include <TimeLib.h>            // https://playground.arduino.cc/Code/Time/ (Margolis 1.5.0)

    bool _slaveModeStatus = true;
    bool _slaveModeOverrideStatus;
    byte _slaveModeFailCount;

#define SLAVE_MODULE_I2C_ADDRESS 105

// Used for timing out on the slave
#define SLAVE_MODE_MAX_RETRIES 20

// -------------------------------------------------------------------------------
#define SLAVE_MODE_100THS               0
#define SLAVE_MODE_DATE                 1
#define SLAVE_MODE_SECS                 2


// ************************************************************
// Called once per second with update info
// ************************************************************
void sendUpdateToSlaveI2C(byte slaveMode, byte dimmingPct) {
  if(_slaveModeStatus) {      

    Wire.beginTransmission(SLAVE_MODULE_I2C_ADDRESS);
    Serial.println("Slave update: " + String(slaveMode) + ", " + String(dimmingPct) + ", " + String(second()) + ", " + String(day()) + ", " + String(month()));
    Wire.write((uint8_t)slaveMode);
    Wire.write((uint8_t)dimmingPct);
    Wire.write((uint8_t)second());
    Wire.write((uint8_t)day());
    Wire.write((uint8_t)month());
    byte error = Wire.endTransmission(true);

    if (error == 0) {
      Serial.println("Sent slave update");
      _slaveModeFailCount = 0;
    } else {
      Serial.println("Failed sending slave update: " + String(error));
      _slaveModeFailCount++;
      if(_slaveModeFailCount > SLAVE_MODE_MAX_RETRIES) {
        Serial.println("Failed to do slave update after " + String(SLAVE_MODE_MAX_RETRIES) + " retries, giving up.");
        _slaveModeStatus = false;
      }
    }
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  Wire.begin();
}

void loop() {
  delay(5000);
  Serial.println("Reset full");
  sendUpdateToSlaveI2C(SLAVE_MODE_100THS, 100);
  sendUpdateToSlaveI2C(SLAVE_MODE_100THS, 100);
  delay(5000);
  Serial.println("Reset dimmed");
  sendUpdateToSlaveI2C(SLAVE_MODE_100THS, 10);
  sendUpdateToSlaveI2C(SLAVE_MODE_100THS, 10);
  delay(5000);
  Serial.println("Seconds full");
  sendUpdateToSlaveI2C(SLAVE_MODE_SECS, 100);
  sendUpdateToSlaveI2C(SLAVE_MODE_SECS, 100);
  delay(5000);
  Serial.println("Seconds dimmed");
  sendUpdateToSlaveI2C(SLAVE_MODE_SECS, 10);
  sendUpdateToSlaveI2C(SLAVE_MODE_SECS, 10);
  delay(5000);
  Serial.println("Blank");
  sendUpdateToSlaveI2C(SLAVE_MODE_SECS, 0);
}
