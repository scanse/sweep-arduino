# sweep-arduino
Arduino Library for Scanse Sweep LiDAR

# Compatibility
Currently the library has only been tested with an Arduino Mega.

# Installation
Copy the entire `Sweep/` folder to your `.../Arduino/libraries/` directory.

# Use

Checkout the provided `Examples/MegaSerialPrinter/MegaSerialPrinter.ino` for a full example.

Include the Sweep library in your sketch:
```arduino
#include <Sweep.h>
...
```

Create a Sweep device with a Stream Object:

```c
// Ex: assuming sweep is connected to Serial #1 (RX1 & TX1) on the Mega
Sweep device(Serial1);
...

```

Adjust device settings:
```c++
// Set the motor speed to 5HZ (codes available from 1->10 HZ)
bool bSuccess = device.setMotorSpeed(MOTOR_SPEED_CODE_5_HZ);
...
// Set the sample rate to 500HZ (codes available for 500, 750 and 1000 HZ)
bool bSuccess = device.setSampleRate(SAMPLE_RATE_CODE_500_HZ);
```

Retrieve current device settings:
```c
// Read the current motor speed
int32_t currentMotorSpeed = device.getMotorSpeed();
if (currentMotorSpeed < 0)
    // Failed to get current motor speed
...

// Read the current sample rate
int32_t currentSampleRate = device.getSampleRate();
if (currentSampleRate < 0)
    // Failed to get current sample rate
...
```

Data collection:
```c

// Start scanning
bool bSuccess = device.startScanning();
...

// Create a scan packet to populate with the sensor reading
ScanPacket reading;
// read 10 individual sensor readings
for (int i = 0; i < 10; i++) {
    if(device.getReading(reading)) {
        bool bFirstOfNewScan = reading.bIsSync;
        float angle = reading.angle;
        uint16_t range = reading.distance;
        uint8_t confidence = reading.signalStrength;
...

// stop scanning
bool bSuccess = device.stopScanning();

```



