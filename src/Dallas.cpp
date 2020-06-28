#include <mgos.h>
#include "Dallas.h"
#include "OnewireInterface.h"

// Model IDs
#define DS18S20MODEL 0x10  // also DS1820
#define DS18B20MODEL 0x28
#define DS1822MODEL 0x22
#define DS1825MODEL 0x3B
#define DS28EA00MODEL 0x42

// OneWire commands
#define STARTCONVO \
  0x44  // Tells device to take a temperature reading and put it on the
        // scratchpad
#define COPYSCRATCH 0x48      // Copy EEPROM
#define READSCRATCH 0xBE      // Read EEPROM
#define WRITESCRATCH 0x4E     // Write to EEPROM
#define RECALLSCRATCH 0xB8    // Reload from last known
#define READPOWERSUPPLY 0xB4  // Determine if device needs parasite power
#define ALARMSEARCH 0xEC      // Query bus for devices with an alarm condition

// Scratchpad locations
#define TEMP_LSB 0
#define TEMP_MSB 1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP 3
#define CONFIGURATION 4
#define INTERNAL_BYTE 5
#define COUNT_REMAIN 6
#define COUNT_PER_C 7
#define SCRATCHPAD_CRC 8

// Device resolution
#define TEMP_9_BIT 0x1F   //  9 bit
#define TEMP_10_BIT 0x3F  // 10 bit
#define TEMP_11_BIT 0x5F  // 11 bit
#define TEMP_12_BIT 0x7F  // 12 bit

Dallas::Dallas()
    : _devices(0),
      _parasite(false),
      _bitResolution(9),
      _waitForConversion(true),
      _checkForConversion(true),
      _ow(NULL),
      _ownOnewire(false) {
}

Dallas::~Dallas() {
  if (_ownOnewire) {
    delete _ow;
  }
}

void Dallas::setOneWire(OnewireInterface *ow) {
  if (_ownOnewire) {
    delete _ow;
    _ow = NULL;
  }
  _ow = ow;
  _devices = 0;
  _parasite = false;
  _bitResolution = 9;
  _waitForConversion = true;
  _checkForConversion = true;
}

/*
 * initialise the bus
 */
void Dallas::begin(void) {
  DeviceAddress deviceAddress;

  _ow->reset_search();
  _devices = 0;  // Reset the number of devices when we enumerate wire devices

  while (_ow->search(deviceAddress)) {
    if (validAddress(deviceAddress)) {
      if (!_parasite && readPowerSupply(deviceAddress)) {
        _parasite = true;
      }
      _bitResolution = MAX(_bitResolution, getResolution(deviceAddress));
      _devices++;
    }
  }
}

bool Dallas::validAddress(const uint8_t *deviceAddress) {
  return (crc8(deviceAddress, 7) == deviceAddress[7]);
}

bool Dallas::validFamily(const uint8_t *deviceAddress) {
  bool ret;
  switch (deviceAddress[0]) {
    case DS18S20MODEL:
    case DS18B20MODEL:
    case DS1822MODEL:
    case DS1825MODEL:
    case DS28EA00MODEL:
      ret = true;
      break;
    default:
      ret = false;
  }
  return ret;
}

/*
 * finds an address at a given index on the bus
 * returns true if the device was found
 */
bool Dallas::getAddress(uint8_t *deviceAddress, uint8_t index) {
  uint8_t depth = 0;

  _ow->reset_search();

  while (depth <= index && _ow->search(deviceAddress)) {
    if (depth == index && validAddress(deviceAddress)) return true;
    depth++;
  }

  return false;
}

/*
 * attempt to determine if the device at the given address is connected to the
 * bus
 */
bool Dallas::isConnected(const uint8_t *deviceAddress) {
  ScratchPad scratchPad;
  return isConnected(deviceAddress, scratchPad);
}

// attempt to determine if the device at the given address is connected to the
// bus
// also allows for updating the read scratchpad

bool Dallas::isConnected(const uint8_t *deviceAddress, uint8_t *scratchPad) {
  bool b = readScratchPad(deviceAddress, scratchPad);
  return b && (crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}

bool Dallas::readScratchPad(const uint8_t *deviceAddress, uint8_t *scratchPad) {
  // send the reset command and fail fast
  int b = _ow->reset();
  if (b == 0) return false;

  _ow->select(deviceAddress);
  _ow->write(READSCRATCH);

  // Read all registers in a simple loop
  // byte 0: temperature LSB
  // byte 1: temperature MSB
  // byte 2: high alarm temp
  // byte 3: low alarm temp
  // byte 4: DS18S20: store for crc
  //         DS18B20 & DS1822: configuration register
  // byte 5: internal use & crc
  // byte 6: DS18S20: COUNT_REMAIN
  //         DS18B20 & DS1822: store for crc
  // byte 7: DS18S20: COUNT_PER_C
  //         DS18B20 & DS1822: store for crc
  // byte 8: SCRATCHPAD_CRC
  _ow->read_bytes(scratchPad, 9);

  b = _ow->reset();
  return (b == 1);
}

void Dallas::writeScratchPad(const uint8_t *deviceAddress,
                             const uint8_t *scratchPad) {
  _ow->reset();
  _ow->select(deviceAddress);
  _ow->write(WRITESCRATCH);
  _ow->write(scratchPad[HIGH_ALARM_TEMP]);  // high alarm temp
  _ow->write(scratchPad[LOW_ALARM_TEMP]);   // low alarm temp

  // DS1820 and DS18S20 have no configuration register
  if (deviceAddress[0] != DS18S20MODEL) {
    _ow->write(scratchPad[CONFIGURATION]);
  }

  //_ow->reset();

  // save the newly written values to eeprom
  //_ow->select(deviceAddress);
  //_ow->write(COPYSCRATCH, parasite);
  // delay(20); // <--- added 20ms delay to allow 10ms long EEPROM write
  // operation (as specified by datasheet)

  // if (parasite) delay(10); // 10ms delay
  _ow->reset();
}

bool Dallas::readPowerSupply(const uint8_t *deviceAddress) {
  bool ret = false;
  _ow->reset();
  _ow->select(deviceAddress);
  _ow->write(READPOWERSUPPLY);
  if (_ow->read_bit() == 0) {
    ret = true;
  }
  _ow->reset();
  return ret;
}

/*
 * returns the current resolution of the device, 9-12
 * returns 0 if device not found
 */
uint8_t Dallas::getResolution(const uint8_t *deviceAddress) {
  // DS1820 and DS18S20 have no resolution configuration register
  if (deviceAddress[0] == DS18S20MODEL) {
    return 12;
  }

  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad)) {
    switch (scratchPad[CONFIGURATION]) {
      case TEMP_12_BIT:
        return 12;
      case TEMP_11_BIT:
        return 11;
      case TEMP_10_BIT:
        return 10;
      case TEMP_9_BIT:
        return 9;
    }
  }
  return 0;
}

/*
 * set resolution of all devices to 9, 10, 11, or 12 bits
 * if new resolution is out of range, it is constrained.
 */
void Dallas::setResolution(uint8_t newResolution) {
  _bitResolution =
      (newResolution < 9) ? 9 : (newResolution > 12 ? 12 : newResolution);
  DeviceAddress deviceAddress;
  for (int i = 0; i < _devices; i++) {
    getAddress(deviceAddress, i);
    setResolution(deviceAddress, _bitResolution, true);
  }
}

/*
 * set resolution of a device to 9, 10, 11, or 12 bits
 * if new resolution is out of range, 9 bits is used.
 */
bool Dallas::setResolution(const uint8_t *deviceAddress, uint8_t newResolution,
                           bool skipGlobalBitResolutionCalculation) {
  /*
   * ensure same behavior as setResolution(uint8_t newResolution)
   */
  newResolution =
      (newResolution < 9) ? 9 : (newResolution > 12 ? 12 : newResolution);

  /*
   * return when stored value == new value
   */
  if (getResolution(deviceAddress) == newResolution) {
    return true;
  }

  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad)) {
    // DS1820 and DS18S20 have no resolution configuration register
    if (deviceAddress[0] != DS18S20MODEL) {
      switch (newResolution) {
        case 12:
          scratchPad[CONFIGURATION] = TEMP_12_BIT;
          break;
        case 11:
          scratchPad[CONFIGURATION] = TEMP_11_BIT;
          break;
        case 10:
          scratchPad[CONFIGURATION] = TEMP_10_BIT;
          break;
        case 9:
        default:
          scratchPad[CONFIGURATION] = TEMP_9_BIT;
          break;
      }
      writeScratchPad(deviceAddress, scratchPad);

      // without calculation we can always set it to max
      _bitResolution = MAX(_bitResolution, newResolution);
      if (!skipGlobalBitResolutionCalculation &&
          (_bitResolution > newResolution)) {
        _bitResolution = newResolution;
        DeviceAddress deviceAddr;
        for (int i = 0; i < _devices; i++) {
          getAddress(deviceAddr, i);
          _bitResolution = MAX(_bitResolution, getResolution(deviceAddr));
        }
      }
    }
    return true;  // new value set
  }

  return false;
}

/*
 * sends command for all devices on the bus to perform a temperature conversion
 */
void Dallas::requestTemperatures() {
  _ow->reset();
  _ow->skip();
  _ow->write(STARTCONVO, _parasite);

  // ASYNC mode?
  if (!_waitForConversion) {
    return;
  }
  blockTillConversionComplete(_bitResolution);
}

/*
 * sends command for one device to perform a temperature by address
 * returns FALSE if device is disconnected
 * returns TRUE  otherwise
 */
bool Dallas::requestTemperaturesByAddress(const uint8_t *deviceAddress) {
  uint8_t bitResolution = getResolution(deviceAddress);
  if (bitResolution == 0) {
    return false;  // Device disconnected
  }

  _ow->reset();
  _ow->select(deviceAddress);
  _ow->write(STARTCONVO, _parasite);

  // ASYNC mode?
  if (!_waitForConversion) {
    return true;
  }

  blockTillConversionComplete(bitResolution);
  return true;
}

/*
 * sends command for one device to perform a temp conversion by index
 */
bool Dallas::requestTemperaturesByIndex(uint8_t deviceIndex) {
  DeviceAddress deviceAddress;
  getAddress(deviceAddress, deviceIndex);

  return requestTemperaturesByAddress(deviceAddress);
}

/*
 * returns temperature in 1/128 degrees C or DEVICE_DISCONNECTED_RAW if the
 * device's scratch pad cannot be read successfully.
 * the numeric value of DEVICE_DISCONNECTED_RAW is defined in
 * dallas_defines.h. It is a large negative number outside the
 * operating range of the device
 */
int16_t Dallas::getTemp(const uint8_t *deviceAddress) {
  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad)) {
    return calculateTemperature(deviceAddress, scratchPad);
  }
  return DEVICE_DISCONNECTED_RAW;
}

/*
 * returns temperature in degrees C or DEVICE_DISCONNECTED_C if the
 * device's scratch pad cannot be read successfully.
 * the numeric value of DEVICE_DISCONNECTED_C is defined in
 * Dallas.h. It is a large negative number outside the
 * operating range of the device
 */
float Dallas::getTempC(const uint8_t *deviceAddress) {
  return rawToCelsius(getTemp(deviceAddress));
}

/*
 * returns temperature in degrees F or DEVICE_DISCONNECTED_F if the
 * device's scratch pad cannot be read successfully.
 * the numeric value of DEVICE_DISCONNECTED_F is defined in
 * Dallas.h. It is a large negative number outside the
 * operating range of the device
 */
float Dallas::getTempF(const uint8_t *deviceAddress) {
  return rawToFahrenheit(getTemp(deviceAddress));
}

/*
 * Fetch temperature for device index
 */
float Dallas::getTempCByIndex(uint8_t deviceIndex) {
  DeviceAddress deviceAddress;
  if (!getAddress(deviceAddress, deviceIndex)) {
    return DEVICE_DISCONNECTED_C;
  }

  return getTempC((uint8_t *) deviceAddress);
}

/*
 * Fetch temperature for device index
 */
float Dallas::getTempFByIndex(uint8_t deviceIndex) {
  DeviceAddress deviceAddress;
  if (!getAddress(deviceAddress, deviceIndex)) {
    return DEVICE_DISCONNECTED_F;
  }

  return getTempF((uint8_t *) deviceAddress);
}

bool Dallas::isConversionComplete() {
  uint8_t b = _ow->read_bit();
  return (b == 1);
}

/*
 * reads scratchpad and returns fixed-point temperature, scaling factor 2^-7
 */
int16_t Dallas::calculateTemperature(const uint8_t *deviceAddress,
                                     uint8_t *scratchPad) {
  int16_t fpTemperature = (((int16_t) scratchPad[TEMP_MSB]) << 11) |
                          (((int16_t) scratchPad[TEMP_LSB]) << 3);

  /*
  DS1820 and DS18S20 have a 9-bit temperature register.
  Resolutions greater than 9-bit can be calculated using the data from
  the temperature, and COUNT REMAIN and COUNT PER °C registers in the
  scratchpad.  The resolution of the calculation depends on the model.
  While the COUNT PER °C register is hard-wired to 16 (10h) in a
  DS18S20, it changes with temperature in DS1820.
  After reading the scratchpad, the TEMP_READ value is obtained by
  truncating the 0.5°C bit (bit 0) from the temperature data. The
  extended resolution temperature can then be calculated using the
  following equation:
                                  COUNT_PER_C - COUNT_REMAIN
  TEMPERATURE = TEMP_READ - 0.25 + --------------------------
                                          COUNT_PER_C
  Hagai Shatz simplified this to integer arithmetic for a 12 bits
  value for a DS18S20, and James Cameron added legacy DS1820 support.
  See -
  http://myarduinotoy.blogspot.co.uk/2013/02/12bit-result-from-ds18s20.html
   */

  if (deviceAddress[0] == DS18S20MODEL) {
    fpTemperature =
        ((fpTemperature & 0xfff0) << 3) - 16 +
        (((scratchPad[COUNT_PER_C] - scratchPad[COUNT_REMAIN]) << 7) /
         scratchPad[COUNT_PER_C]);
  }

  return fpTemperature;
}

/*
 * Continue to check if the IC has responded with a temperature
 */
void Dallas::blockTillConversionComplete(uint8_t bitResolution) {
  uint32_t delms = 1000 * millisToWaitForConversion(bitResolution);
  if (_checkForConversion && !_parasite) {
    uint64_t now = (uint64_t)(mgos_uptime() * 1000 * 1000);
    while (!isConversionComplete() &&
           ((uint64_t)(mgos_uptime() * 1000 * 1000) - delms < now))
      ;
  } else {
    mgos_usleep(delms);
  }
}

/*
 *  returns number of milliseconds to wait till conversion is complete (based on
 * IC datasheet)
 */
int16_t Dallas::millisToWaitForConversion(uint8_t bitResolution) {
  switch (bitResolution) {
    case 9:
      return 94;
    case 10:
      return 188;
    case 11:
      return 375;
    default:
      return 750;
  }
}

/*
 * Convert from Celsius to Fahrenheit
 */
float Dallas::toFahrenheit(float celsius) {
  return (celsius * 1.8) + 32;
}

/*
 * Convert from Fahrenheit to Celsius
 */
float Dallas::toCelsius(float fahrenheit) {
  return (fahrenheit - 32) * 0.555555556;
}

/*
 *  convert from raw to Celsius
 */
float Dallas::rawToCelsius(int16_t raw) {
  if (raw <= DEVICE_DISCONNECTED_RAW) {
    return DEVICE_DISCONNECTED_C;
  }
  // C = RAW/128
  return (float) raw * 0.0078125;
}

/*
 *  convert from raw to Fahrenheit
 */
float Dallas::rawToFahrenheit(int16_t raw) {
  if (raw <= DEVICE_DISCONNECTED_RAW) {
    return DEVICE_DISCONNECTED_F;
  }
  // C = RAW/128
  // F = (C*1.8)+32 = (RAW/128*1.8)+32 = (RAW*0.0140625)+32
  // return ((float) raw * 0.0140625) + 32;
  return ((float) raw * 1.8 / 128.0) + 32.0;
}

// This table comes from Dallas sample code where it is freely reusable,
// though Copyright (C) 2000 Dallas Semiconductor Corporation
static const uint8_t crc_table[] = {
    0,   94,  188, 226, 97,  63,  221, 131, 194, 156, 126, 32,  163, 253, 31,
    65,  157, 195, 33,  127, 252, 162, 64,  30,  95,  1,   227, 189, 62,  96,
    130, 220, 35,  125, 159, 193, 66,  28,  254, 160, 225, 191, 93,  3,   128,
    222, 60,  98,  190, 224, 2,   92,  223, 129, 99,  61,  124, 34,  192, 158,
    29,  67,  161, 255, 70,  24,  250, 164, 39,  121, 155, 197, 132, 218, 56,
    102, 229, 187, 89,  7,   219, 133, 103, 57,  186, 228, 6,   88,  25,  71,
    165, 251, 120, 38,  196, 154, 101, 59,  217, 135, 4,   90,  184, 230, 167,
    249, 27,  69,  198, 152, 122, 36,  248, 166, 68,  26,  153, 199, 37,  123,
    58,  100, 134, 216, 91,  5,   231, 185, 140, 210, 48,  110, 237, 179, 81,
    15,  78,  16,  242, 172, 47,  113, 147, 205, 17,  79,  173, 243, 112, 46,
    204, 146, 211, 141, 111, 49,  178, 236, 14,  80,  175, 241, 19,  77,  206,
    144, 114, 44,  109, 51,  209, 143, 12,  82,  176, 238, 50,  108, 142, 208,
    83,  13,  239, 177, 240, 174, 76,  18,  145, 207, 45,  115, 202, 148, 118,
    40,  171, 245, 23,  73,  8,   86,  180, 234, 105, 55,  213, 139, 87,  9,
    235, 181, 54,  104, 138, 212, 149, 203, 41,  119, 244, 170, 72,  22,  233,
    183, 85,  11,  136, 214, 52,  106, 43,  117, 151, 201, 74,  20,  246, 168,
    116, 42,  200, 150, 21,  75,  169, 247, 182, 232, 10,  84,  215, 137, 107,
    53};

uint8_t Dallas::crc8(const uint8_t *addr, uint8_t len) {
  uint8_t res = 0x00;
  while (len-- > 0) {
    res = crc_table[res ^ *addr++];
  }
  return res;
}
