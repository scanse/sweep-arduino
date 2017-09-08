#include "ScanPacket.h"

ScanPacket::ScanPacket(const bool bIsSync, const uint16_t rawAngle,
    const uint16_t distance, const uint8_t signalStrength):
    _bIsSync(bIsSync), _rawAngle(rawAngle), _distance(distance),
    _signalStrength(signalStrength) {}

bool ScanPacket::isSync() const
{
    return _bIsSync;
}

float ScanPacket::getAngleDegrees() const
{
    return _rawAngle / SCALING_FACTOR;
}

uint16_t ScanPacket::getAngleRaw() const
{
    return _rawAngle;
}

uint16_t ScanPacket::getDistanceCentimeters() const
{
    return _distance;
}

uint8_t ScanPacket::getSignalStrength() const
{
    return _signalStrength;
}

float ScanPacket::getNormalizedSignalStrength() const
{
    return _signalStrength / 255;
}
