#ifndef _SCANPACKET_H
#define _SCANPACKET_H

#include <stdint.h>

/**
*   Class representing the data block returned from the Sweep.
*/
class ScanPacket
{
  public:
    // Constructor
    ScanPacket(const bool bIsSync, const uint16_t rawAngle,
        const uint16_t distance, const uint8_t signalStrength);

    // Returns true if this reading is the first of a new scan
    bool isSync() const;

    // Returns the angle in degrees as a float
    float getAngleDegrees() const;

    // Returns the angle as a raw fixed point value
    uint16_t getAngleRaw() const;

    // Returns the range measurement in centimeters
    uint16_t getDistanceCentimeters() const;

    // Returns the signal strength as an integer value between 0 and 255
    uint8_t getSignalStrength() const;

    // Returns the signal strength as a float normalized between 0 and 1
    float getNormalizedSignalStrength() const;

  private:
    // Scaling factor of raw angle
    static constexpr float SCALING_FACTOR = 16.0f;

    bool _bIsSync;           // 1 -> first reading of new scan, 0 otherwise
    uint16_t _rawAngle;      // fixed point value: (degrees * 16)
    uint16_t _distance;      // cm
    uint8_t _signalStrength; // 0:255, higher is better
};

#endif
