
/* 
  Scanse Sweep Arduino Library Examples
  
  MegaSerialPrinter:
      - Example sketch for using the Scanse Sweep with the Arduino Mega 2560.
        Collects 3 complete scans, and then prints the sensor readings
      - Assumes Sweep sensor is physically connected to Serial #1 (RX1 & TX1)
        - For the sweep's power, ground, RX & TX pins, follow the connector 
          pinouts in the sweep user manual located here: 
          http://scanse.io/downloads
        - Be sure to connect RX_device -> TX_arduino & TX_device -> RX_arduino
      - You should run the sketch off USB power alone! Using both power sources 
        (USB + power jack) can cause issues. 
      - Note that running off of USB power is not entirely adequate for the sweep, 
        so the quantity and qaulity of sensor readings will drop. This is OK for
        this example , as it is only meant to provide some visual feedback over 
        the serial monitor.
      - In your own projects that do not involve the serial monitor, be sure to use
        dedicated power instead of the USB.

  Created by Scanse LLC, February 21, 2017.
  Released into the public domain.
*/

#include <Sweep.h>

// Create a Sweep device using Serial #1 (RX1 & TX1)
Sweep device(Serial1);
// Scan packet struct, used to store info for a single reading
ScanPacket reading;

// keeps track of how many scans have been collected
uint8_t scanCount = 0;
// keeps track of how many samples have been collected
uint16_t sampleCount = 0;

// Arrays to store attributes of collected scans
bool syncValues[500];         // 1 -> first reading of new scan, 0 otherwise
float angles[500];            // in degrees (accurate to the millidegree)
uint16_t distances[500];      // in cm
uint8_t signalStrengths[500]; // 0:255, higher is better

// Finite States for the program sequence
const uint8_t STATE_WAIT_FOR_USER_INPUT = 0;
const uint8_t STATE_ADJUST_DEVICE_SETTINGS = 1;
const uint8_t STATE_VERIFY_CURRENT_DEVICE_SETTINGS = 2;
const uint8_t STATE_BEGIN_DATA_ACQUISITION = 3;
const uint8_t STATE_GATHER_DATA = 4;
const uint8_t STATE_STOP_DATA_ACQUISITION = 5;
const uint8_t STATE_REPORT_COLLECTED_DATA = 6;
const uint8_t STATE_RESET = 7;
const uint8_t STATE_ERROR = 8;

// Current state in the program sequence
uint8_t currentState;

// String to collect user input over serial
String userInput = "";

void setup()
{
  // Initialize serial
  Serial.begin(9600);    // serial terminal on the computer
  Serial1.begin(115200); // sweep device

  // reserve space to accumulate user message
  userInput.reserve(50);

  // initialize counter variables and reset the current state
  reset();
}

// Loop functions as an FSM (finite state machine)
void loop()
{
  switch (currentState)
  {
  case STATE_WAIT_FOR_USER_INPUT:
    if (listenForUserInput())
      currentState++;
    break;
  case STATE_ADJUST_DEVICE_SETTINGS:
    currentState = adjustDeviceSettings() ? currentState + 1 : STATE_ERROR;
    break;
  case STATE_VERIFY_CURRENT_DEVICE_SETTINGS:
    currentState = verifyCurrentDeviceSettings() ? currentState + 1 : STATE_ERROR;
    break;
  case STATE_BEGIN_DATA_ACQUISITION:
    currentState = beginDataCollectionPhase() ? currentState + 1 : STATE_ERROR;
    break;
  case STATE_GATHER_DATA:
    gatherSensorReading();
    if (scanCount > 3)
      currentState++;
    break;
  case STATE_STOP_DATA_ACQUISITION:
    currentState = stopDataCollectionPhase() ? currentState + 1 : STATE_ERROR;
    break;
  case STATE_REPORT_COLLECTED_DATA:
    printCollectedData();
    currentState++;
    break;
  case STATE_RESET:
    reset();
    currentState = 0;
    break;
  default: // there was some error
    // DO NOTHING
    break;
  }
}

// checks if the user has communicated anything over serial
// looks for the user to send "start"
bool listenForUserInput()
{
  while (Serial.available())
  {
    userInput += (char)Serial.read();
  }
  if (userInput.indexOf("start") != -1)
  {
    Serial.println("Registered user start.");
    return true;
  }
  return false;
}

// Adjusts the device settings
bool adjustDeviceSettings()
{
  // Set the motor speed to 5HZ (codes available from 1->10 HZ)
  bool bSuccess = device.setMotorSpeed(MOTOR_SPEED_CODE_5_HZ);
  Serial.println(bSuccess ? "\nSuccessfully set motor speed." : "\nFailed to set motor speed");

  /*  
  // Device will always default to 500HZ scan rate when it is powered on.
  // Snippet below is left for reference.
  // Set the sample rate to 500HZ (codes available for 500, 750 and 1000 HZ)
  bool bSuccess = device.setSampleRate(SAMPLE_RATE_CODE_500_HZ);
  Serial.println(bSuccess ? "\nSuccessfully set sample rate." : "\nFailed to set sample rate.");
*/
  return bSuccess;
}

// Querries the current device settings (motor speed and sample rate)
// and prints them to the console
bool verifyCurrentDeviceSettings()
{
  // Read the current motor speed and sample rate
  int32_t currentMotorSpeed = device.getMotorSpeed();
  if (currentMotorSpeed < 0)
  {
    Serial.println("\nFailed to get current motor speed");
    return false;
  }
  int32_t currentSampleRate = device.getSampleRate();
  if (currentSampleRate < 0)
  {
    Serial.println("\nFailed to get current sample rate");
    return false;
  }

  // Report the motor speed and sample rate to the computer terminal
  Serial.println("\nMotor Speed Setting: " + String(currentMotorSpeed) + " HZ");
  Serial.println("Sample Rate Setting: " + String(currentSampleRate) + " HZ");

  return true;
}

// Initiates the data collection phase (begins scanning)
bool beginDataCollectionPhase()
{
  // Attempt to start scanning
  Serial.println("\nWaiting for motor speed to stabilize and calibration routine to complete...");
  bool bSuccess = device.startScanning();
  Serial.println(bSuccess ? "\nSuccessfully initiated scanning..." : "\nFailed to start scanning.");
  if (bSuccess)
    Serial.println("\nGathering 3 scans...");
  return bSuccess;
}

// Gathers individual sensor readings until 3 complete scans have been collected
void gatherSensorReading()
{
  // attempt to get the next scan packet
  // Note: getReading() will write values into the "reading" variable
  if (device.getReading(reading))
  {
    // check if this reading was the very first reading of a new 360 degree scan
    if (reading.bIsSync)
      scanCount++;

    // don't collect more than 3 scans
    if (scanCount > 3)
      return;

    // store the info for this sample
    syncValues[sampleCount] = reading.bIsSync;
    angles[sampleCount] = reading.angle;
    distances[sampleCount] = reading.distance;
    signalStrengths[sampleCount] = reading.signalStrength;

    // increment sample count
    sampleCount++;
  }
}

// Terminates the data collection phase (stops scanning)
bool stopDataCollectionPhase()
{
  // Attempt to stop scanning
  bool bSuccess = device.stopScanning();

  Serial.println(bSuccess ? "\nSuccessfully stopped scanning." : "\nFailed to stop scanning.");
  return bSuccess;
}

// Prints the collected data to the console
// (only prints the complete scans, ignores the first partial)
void printCollectedData()
{
  Serial.println("\nPrinting info for the collected scans (NOT REAL-TIME):");

  int indexOfFirstSyncReading = 0;
  // don't print the trailing readings from the first partial scan
  while (!syncValues[indexOfFirstSyncReading])
  {
    indexOfFirstSyncReading++;
  }
  // print the readings for all the complete scans
  for (int i = indexOfFirstSyncReading; i < sampleCount; i++)
  {
    if (syncValues[i])
    {
      Serial.println("\n----------------------NEW SCAN----------------------");
    }
    Serial.println("Angle: " + String(angles[i], 3) + ", Distance: " + String(distances[i]) + ", Signal Strength: " + String(signalStrengths[i]));
  }
}

// Resets the variables and state so the sequence can be repeated
void reset()
{
  scanCount = 0;
  sampleCount = 0;
  Serial.flush();
  userInput = "";
  Serial.println("\n\nWhenever you are ready, type \"start\" to to begin the sequence...");
  currentState = 0;
}