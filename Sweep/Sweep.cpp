#include "Sweep.h"

Sweep::Sweep(Stream &serial) : _serial(serial)
{
    bIsScanning = false;
}
Sweep::~Sweep()
{
    _serial.flush();
}

bool Sweep::startScanning()
{
    if (bIsScanning)
        return false;

    // wait until the device is ready (calibration complete + motor stabilized)
    if (!waitUntilMotorReady())
        return false;

    _writeCommand(_DATA_ACQUISITION_START);
    // wait for the receipt (possible timeout)
    if (_readResponseHeader())
    {
        // TODO: validate receipt
        bIsScanning = true;
        return true;
    }
    return false;
}

bool Sweep::stopScanning()
{
    _writeCommand(_DATA_ACQUISITION_STOP);

    // wait for the device to stop sending packets
    delay(500);

    // then flush the buffer and send STOP again to check for a receipt
    _flushInputBuffer();
    _writeCommand(_DATA_ACQUISITION_STOP);

    // wait for the receipt (possible timeout)
    if (_readResponseHeader())
    {
        // TODO: validate receipt
        bIsScanning = false;
        return true;
    }
    return false;
}

bool Sweep::getReading(ScanPacket &reading)
{
    if (!bIsScanning)
        return false;

    // wait for the receipt (possible timeout)
    if (_readResponseScanPacket())
    {
        // TODO: validate receipt
        reading.bIsSync = _responseScanPacket[0] % 2 == 1;
        reading.rawAngle = _responseScanPacket[2] << 8;
        reading.rawAngle += _responseScanPacket[1];
        // convert the angle into a float in degrees
        reading.angle = angle_raw_to_deg((_responseScanPacket[2] << 8) + (_responseScanPacket[1]));
        reading.distance = (_responseScanPacket[4] << 8) + (_responseScanPacket[3]);
        reading.signalStrength = _responseScanPacket[5];
        return true;
    }
    return false;
}

bool Sweep::getMotorReady()
{
    if (bIsScanning)
        return false;
    _writeCommand(_MOTOR_READY);
    if (_readResponseInfoSetting())
    {
        // TODO: validate receipt (hold off until performance hit is determined)
        const uint8_t readyCode[2] = {_responseInfoSetting[2], _responseInfoSetting[3]};
        // readyCode == 0 indicates device is ready
        return _ascii_bytes_to_integer(readyCode) == 0;
    }
    else
    {
      return false;
    }
}

bool Sweep::waitUntilMotorReady()
{
    if (bIsScanning)
        return false;
    // only check for 10 seconds (20 iterations with 500ms pause)
    for (uint8_t i = 0; i < 20; ++i)
    {
        if (getMotorReady())
            return true;
        delay(500);
    }
    // timeout after 10 seconds
    return false;
}

int32_t Sweep::getMotorSpeed()
{
    if (bIsScanning)
        return false;

    _writeCommand(_MOTOR_INFORMATION);
    if (_readResponseInfoSetting())
    {
        // TODO: validate receipt (hold off until performance hit is determined)
        const uint8_t speedCode[2] = {_responseInfoSetting[2], _responseInfoSetting[3]};
        return _ascii_bytes_to_integer(speedCode);
    }
    return -1;
}

bool Sweep::setMotorSpeed(const uint8_t motorSpeedCode[2])
{
    if (bIsScanning)
        return false;

    // wait until the device is ready (calibration complete + motor stabilized)
    if (!waitUntilMotorReady())
        return false;

    _writeCommandWithArgument(_MOTOR_SPEED_ADJUST, motorSpeedCode);
    // wait for the receipt (possible timeout)
    if (_readResponseParam())
    {
        // TODO: validate receipt
        return true;
    }
    return false;
}

int32_t Sweep::getSampleRate()
{
    if (bIsScanning)
        return false;

    _writeCommand(_SAMPLE_RATE_INFORMATION);
    if (_readResponseInfoSetting())
    {
        // TODO: validate receipt (hold off until performance hit is determined)
        const uint8_t sampleRateCode[2] = {_responseInfoSetting[2], _responseInfoSetting[3]};
        switch (_ascii_bytes_to_integer(sampleRateCode))
        {
        case 1:
            return 500;
        case 2:
            return 750;
        case 3:
            return 1000;
        default:
            break;
        }
    }
    return -1;
}

bool Sweep::setSampleRate(const uint8_t sampleRateCode[2])
{
    if (bIsScanning)
        return false;

    _writeCommandWithArgument(_SAMPLE_RATE_ADJUST, sampleRateCode);
    // wait for the receipt (possible timeout)
    if (_readResponseParam())
    {
        // TODO: validate receipt
        return true;
    }
    return false;
}

void Sweep::reset()
{
    _writeCommand(_RESET_DEVICE);
}

void Sweep::_writeCommand(const uint8_t cmd[2])
{
    const uint8_t command[3] = {cmd[0], cmd[1], _COMMAND_TERMINATION};

    _serial.write(command, 3);
}

void Sweep::_writeCommandWithArgument(const uint8_t cmd[2], const uint8_t arg[2])
{
    const uint8_t command[5] = {cmd[0], cmd[1], arg[0], arg[1], _COMMAND_TERMINATION};

    _serial.write(command, 5);
}

bool Sweep::_readResponseHeader()
{
    // determine the expected number of bytes to read
    uint8_t len = sweepArrLen(_responseHeader);

    // set a timeout on the read
    _serial.setTimeout(1000);

    // attempt to read (can timeout)
    int numBytesRead = _serial.readBytes(_responseHeader, len);

    // return true if the expected num of bytes were read
    return numBytesRead == len;
}

bool Sweep::_readResponseParam()
{
    // determine the expected number of bytes to read
    uint8_t len = sweepArrLen(_responseParam);

    // set a timeout on the read
    _serial.setTimeout(1000);

    // attempt to read (can timeout)
    int numBytesRead = _serial.readBytes(_responseParam, len);

    // return true if the expected num of bytes were read
    return numBytesRead == len;
}

bool Sweep::_readResponseScanPacket()
{
    // determine the expected number of bytes to read
    uint8_t len = sweepArrLen(_responseScanPacket);

    // set a timeout on the read
    _serial.setTimeout(1000);

    // attempt to read (can timeout)
    int numBytesRead = _serial.readBytes(_responseScanPacket, len);

    // return true if the expected num of bytes were read
    return numBytesRead == len;
}

bool Sweep::_readResponseInfoDevice()
{
    // determine the expected number of bytes to read
    uint8_t len = sweepArrLen(_responseInfoDevice);

    // set a timeout on the read
    _serial.setTimeout(1000);

    // attempt to read (can timeout)
    int numBytesRead = _serial.readBytes(_responseInfoDevice, len);

    // return true if the expected num of bytes were read
    return numBytesRead == len;
}

bool Sweep::_readResponseInfoVersion()
{
    // determine the expected number of bytes to read
    uint8_t len = sweepArrLen(_responseInfoVersion);

    // set a timeout on the read
    _serial.setTimeout(1000);

    // attempt to read (can timeout)
    int numBytesRead = _serial.readBytes(_responseInfoVersion, len);

    // return true if the expected num of bytes were read
    return numBytesRead == len;
}

bool Sweep::_readResponseInfoSetting()
{
    // determine the expected number of bytes to read
    uint8_t len = sweepArrLen(_responseInfoSetting);

    // set a timeout on the read
    _serial.setTimeout(1000);

    // attempt to read (can timeout)
    int numBytesRead = _serial.readBytes(_responseInfoSetting, len);

    // return true if the expected num of bytes were read
    return numBytesRead == len;
}

void Sweep::_flushInputBuffer()
{
    while (_serial.available() > 0)
    {
        _serial.read();
    }
}
