/*
    Sweep.h - Library for communicating with Scanse Sweep scanning LiDAR device.
    Created by David C. Young, February 21, 2017.
    Released into the public domain.
*/
#ifndef _SWEEP_H
#define _SWEEP_H

#include <Arduino.h>

// Returns the num of elements in an array
#define sweepArrLen(x) (sizeof(x) / sizeof(*x))

// Available Motor Speed Codes for the setMotorSpeed method
const uint8_t MOTOR_SPEED_CODE_1_HZ[2] = {'0', '1'};
const uint8_t MOTOR_SPEED_CODE_2_HZ[2] = {'0', '2'};
const uint8_t MOTOR_SPEED_CODE_3_HZ[2] = {'0', '3'};
const uint8_t MOTOR_SPEED_CODE_4_HZ[2] = {'0', '4'};
const uint8_t MOTOR_SPEED_CODE_5_HZ[2] = {'0', '5'};
const uint8_t MOTOR_SPEED_CODE_6_HZ[2] = {'0', '6'};
const uint8_t MOTOR_SPEED_CODE_7_HZ[2] = {'0', '7'};
const uint8_t MOTOR_SPEED_CODE_8_HZ[2] = {'0', '8'};
const uint8_t MOTOR_SPEED_CODE_9_HZ[2] = {'0', '9'};
const uint8_t MOTOR_SPEED_CODE_10_HZ[2] = {'1', '0'};

// Available Sample Rate Codes for the setSampleRate method
const uint8_t SAMPLE_RATE_CODE_500_HZ[2] = {'0', '1'};
const uint8_t SAMPLE_RATE_CODE_750_HZ[2] = {'0', '2'};
const uint8_t SAMPLE_RATE_CODE_1000_HZ[2] = {'0', '3'};

/**
*   Structure representing a single sensor reading
*         
*   bIsSync: Whether or not this reading is the first of a new scan
*   angle: Azimuth at which the reading was recorded
*   distance: Range at which the reading was recorded
*   signalStrength: Strength of the reading (confidence)
*/
struct ScanPacket
{
    bool bIsSync;           // 1 -> first reading of new scan, 0 otherwise
    float angle;            // degrees
    uint16_t distance;      // cm
    uint8_t signalStrength; // 0:255, higher is better
};

/**
*   Class representing a sweep device
*/
class Sweep
{
  public:
    // Constructor
    Sweep(Stream &serial);

    // Destructor
    ~Sweep();

    // True if the device is currently scanning
    bool bIsScanning;

    /**
    *   Initiates an active data stream that sends scan packets
    *   (each representing a single reading) over the _serial 
    *   stream. This data stream continues indefinitely until
    *   "stopScanning()" is called.
    *
    *   @return True if start was successful
    */
    bool startScanning();

    /**
    *   Terminates an active data stream.
    *
    *   @return True if stop was successful
    */
    bool stopScanning();

    /**
    *   Reads a single sensor reading from the serial buffer.
    *   (must be called frequently to keep up with the data stream)
    *
    *   @param reading A reference to a ScanPacket where the 
    *       values for the reading will be placed.
    *   @return True if the read was successful.
    */
    bool getReading(ScanPacket &reading);

    /**
    *   Check if the device is ready. A device is ready if the
    *   calibration routine is complete and the motor speed has 
    *   stabilized to the current speed setting.
    *
    *   @return True if the device is ready
    */
    bool getMotorReady();

    /**
    *   Waits (blocks) until the device is ready, or the wait 
    *   times out after 8 seconds. See getMotorReady for details.
    *
    *   @return True if motor is ready, false if the wait timed out
    */
    bool waitUntilMotorReady();

    // Returns the current motor speed setting in HZ
    int32_t getMotorSpeed();

    /**
    *   Adjusts the motor speed setting to the provided code.
    *   (Recommend users pass const codes defined by library)
    *
    *   @param motorSpeedCode ASCII code between '00' and '10'
    *   @return True if the setting was successfully applied.
    */
    bool setMotorSpeed(const uint8_t motorSpeedCode[2]);

    // Returns the current sample rate setting in HZ
    int32_t getSampleRate();

    /**
    *   Adjusts the sample rate setting to the provided code.
    *   (Recommend users pass const codes defined by library)
    *
    *   @param sampleRateCode ASCII code between '01' and '03'
    *   @return True if the setting was successfully applied.
    */
    bool setSampleRate(const uint8_t sampleRateCode[2]);

    /**
    *   Resets the device. 
    *   Warning: attempting to communicate with the device while
    *           it is resetting will cause issues. Device is ready
    *           when the LED turns blue.
    */
    void reset();

  private:
    // The stream object connected to the sweep device.
    Stream &_serial;

    // Command Prefixes (See Sweep User Manual for CommProtocol)
    const uint8_t _COMMAND_TERMINATION = '\n';
    const uint8_t _DATA_ACQUISITION_START[2] = {'D', 'S'};
    const uint8_t _DATA_ACQUISITION_STOP[2] = {'D', 'X'};
    const uint8_t _MOTOR_READY[2] = {'M', 'Z'};
    const uint8_t _MOTOR_SPEED_ADJUST[2] = {'M', 'S'};
    const uint8_t _MOTOR_INFORMATION[2] = {'M', 'I'};
    const uint8_t _SAMPLE_RATE_ADJUST[2] = {'L', 'R'};
    const uint8_t _SAMPLE_RATE_INFORMATION[2] = {'L', 'I'};
    const uint8_t _VERSION_INFORMATION[2] = {'I', 'V'};
    const uint8_t _DEVICE_INFORMATION[2] = {'I', 'D'};
    const uint8_t _RESET_DEVICE[2] = {'R', 'R'};

    // Arrays to hold responses
    uint8_t _responseHeader[6];
    uint8_t _responseParam[9];
    uint8_t _responseScanPacket[7];
    uint8_t _responseInfoDevice[18];
    uint8_t _responseInfoVersion[21];
    uint8_t _responseInfoSetting[5];

    // Write command without any argument
    bool _writeCommand(const uint8_t cmd[2]);
    // Write command with a 2 character argument code
    bool _writeCommandWithArgument(const uint8_t cmd[2], const uint8_t arg[2]);

    // Read various types of responses
    bool _readResponseHeader();
    bool _readResponseParam();
    bool _readResponseScanPacket();
    bool _readResponseInfoDevice();
    bool _readResponseInfoVersion();
    bool _readResponseInfoSetting();
    void _flushInputBuffer();

    // Some protocol conversion utilities
    inline int angle_raw_to_deg(uint16_t v)
    {
        // angle is transmitted as fixed point integer with scaling factor of 16
        return static_cast<int>(v / 16.0f);
    }

    // converts a pair of ascii code (between '00':'10') into an integer
    inline const int32_t _ascii_bytes_to_integer(const uint8_t bytes[2])
    {
        const uint8_t ASCIINumberBlockOffset = 48;

        const uint8_t num1 = bytes[0] - ASCIINumberBlockOffset;
        const uint8_t num2 = bytes[1] - ASCIINumberBlockOffset;

        if (num1 > 9 || num2 > 9 || num1 < 0 || num2 < 0)
            return -1;

        return (num1 * 10) + (num2 * 1);
    }
};

#endif