# Changelog

## 9/7/2017
- Updated `README`
- Updated `keywords.txt`
- Expanded `ScanPacket` struct into full class
  - `ScanPacket` is now immutable, and its data is accessed through getter methods
  - Obtaining a reading now requires a separate boolean variable for the success flag
  - Angle data is efficiently stored in raw 16-bit fixed point value
  - Getter methods provide an interface to obtain the angle as a raw value or as a float in degrees
- Removed unused method `angle_raw_to_deg` from `Sweep.h`