# sweep-arduino
Arduino Library for Scanse Sweep LiDAR

# Compatibility
Currently the library has only been tested with an Arduino Mega.

# Installation
Copy the entire `Sweep/` folder to your `.../Arduino/libraries/` directory.

# Use

Checkout the provided `Examples/MegaSerialPrinter/MegaSerialPrinter.ino` for a full example.

![Alt text](wiring_diagrams/MegaSerialPrinter.png?raw=true "Title")



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


# API

```c++
Sweep(Stream &serial)
```

Constructs a sweep device on the provided Stream object.

```c++
bool bIsScanning
```

True if the device is currently scanning.

```c++
bool startScanning()
```
Signals the sweep device to start scanning. Initiates an indefinite stream of individual sensor readings until `stopScanning()` is called. 
During an active data stream, any attempt to communicate with the device other than `stopScanning()` will fail.


```c++
bool stopScanning()
```

Signals the sweep device to stop scanning.

```c++
bool getReading(ScanPacket &reading)
```

Reads a single sensor reading from the serial buffer. Must be called frequently enough to keep up with the data stream.

```c++
struct ScanPacket
{
    bool bIsSync;           // 1 -> first reading of new scan, 0 otherwise
    float angle;            // degrees
    uint16_t distance;      // cm
    uint8_t signalStrength; // 0:255, higher is better
}
```

Structure representing a single sensor reading (ranging). ie: a full 360deg scan would be composed of many such readings.

```c++
int32_t getMotorSpeed()
```

Returns the current motor speed setting in HZ

```c++
bool setMotorSpeed(const uint8_t motorSpeedCode[2])

// Available Codes
MOTOR_SPEED_CODE_1_HZ
MOTOR_SPEED_CODE_2_HZ
MOTOR_SPEED_CODE_3_HZ
MOTOR_SPEED_CODE_4_HZ
MOTOR_SPEED_CODE_5_HZ
MOTOR_SPEED_CODE_6_HZ
MOTOR_SPEED_CODE_7_HZ
MOTOR_SPEED_CODE_8_HZ
MOTOR_SPEED_CODE_9_HZ
MOTOR_SPEED_CODE_10_HZ
```

Adjusts the motor speed setting to the provided code. Recommend users pass one of the const codes defined by library:

```c++
int32_t getSampleRate()
```

Returns the current sample rate setting in HZ.

```c++
bool setSampleRate(const uint8_t sampleRateCode[2])

// Available Codes
SAMPLE_RATE_CODE_500_HZ
SAMPLE_RATE_CODE_750_HZ
SAMPLE_RATE_CODE_1000_HZ
```

Adjusts the sample rate setting to the provided code. Recommend users pass one of the const codes defined by library: