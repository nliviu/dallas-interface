#pragma once

#ifdef __cplusplus
#include "Dallas.h"
#else
typedef struct DallasTag Dallas;
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Initializes the mgos_dallas_ driver with a GPIO `pin`
 * Return value: handle opaque pointer.
 */
//Dallas* mgos_dallas_create(uint8_t pin);

/*
 * Destructor
 * Close mgos_dallas_ handle. Return value: none.
 */
void mgos_dallas_close(Dallas* dt);

/*
 *  Initialises the 1-Wire bus.
 */
void mgos_dallas_begin(Dallas *dt);

/*
 * Returns the number of devices found on the bus.
 * Return always 0 if an operaiton failed.
 */
int mgos_dallas_get_device_count(Dallas *dt);

/*
 * Returns true if address is valid.
 * Return always false if an operaiton failed.
 */
bool mgos_dallas_valid_address(Dallas *dt, const uint8_t *addr);

/*
 * Returns true if address is of the family of sensors the lib supports.
 * Return always false if an operaiton failed.
 */
bool mgos_dallas_valid_family(Dallas *dt, const uint8_t *addr);

/*
 * Finds an address at a given index on the bus.
 * Return false if the device was not found or an operaiton failed.
 * Returns true otherwise.
 */
bool mgos_dallas_get_address(Dallas *dt, uint8_t *addr, int idx);

/*
 * Attempt to determine if the device at the given address is connected to the bus.
 * Return false if the device is not connected or an operaiton failed.
 * Returns true otherwise.
 */
bool mgos_dallas_is_connected(Dallas *dt, const uint8_t *addr);

/*
 * Attempts to determine if the device at the given address is connected to the bus.
 * Also allows for updating the read scratchpad.
 * Return false if the device is not connected or an operaiton failed.
 * Returns true otherwise.
 */
bool mgos_dallas_is_connected_sp(Dallas *dt, const uint8_t *addr, uint8_t *sp);

/*
 * Reads device's scratchpad.
 * Returns false if an operaiton failed.
 * Returns true otherwise.
 */
bool mgos_dallas_read_scratch_pad(Dallas *dt, const uint8_t *addr, uint8_t *sp);

/*
 * Writes device's scratchpad.
 */
void mgos_dallas_write_scratch_pad(Dallas *dt, const uint8_t *addr, const uint8_t *sp);

/*
 * Read device's power requirements.
 * Return true if device needs parasite power.
 * Return always false if an operaiton failed.
 */
bool mgos_dallas_read_power_supply(Dallas *dt, const uint8_t *addr);

/*
 * Get global resolution.
 */
int mgos_dallas_get_global_resolution(Dallas *dt);

/*
 * Set global resolution to 9, 10, 11, or 12 bits.
 */
void mgos_dallas_set_global_resolution(Dallas *dt, int res);

/*
 * Returns the device resolution: 9, 10, 11, or 12 bits.
 * Returns 0 if device not found or if an operation failed.
 */
int mgos_dallas_get_resolution(Dallas *dt, const uint8_t *addr);

/*
 * Set resolution of a device to 9, 10, 11, or 12 bits.
 * If new resolution is out of range, 9 bits is used.
 * Return true if a new value was stored.
 * Returns false otherwise.
 */
bool mgos_dallas_set_resolution(Dallas *dt, const uint8_t *addr, int res, bool skip_global_calc);

/*
 * Sets the waitForConversion flag.
 */
void mgos_dallas_set_wait_for_conversion(Dallas *dt, bool f);

/*
 * Gets the value of the waitForConversion flag.
 * Return always false if an operaiton failed.
 */
bool mgos_dallas_get_wait_for_conversion(Dallas *dt);

/*
 * Sets the checkForConversion flag.
 */
void mgos_dallas_set_check_for_conversion(Dallas *dt, bool f);

/*
 * Gets the value of the waitForConversion flag.
 * Return always false if an operaiton failed.
 */
bool mgos_dallas_get_check_for_conversion(Dallas *dt);

/*
 * Sends command for all devices on the bus to perform a temperature conversion.
 * Returns false if a device is disconnected or if an operaiton failed.
 * Returns true otherwise.
 */
void mgos_dallas_request_temperatures(Dallas *dt);

/*
 * Sends command for one device to perform a temperature conversion by address.
 * Returns false if a device is disconnected or if an operaiton failed.
 * Returns true otherwise.
 */
bool mgos_dallas_request_temperatures_by_address(Dallas *dt, const uint8_t *addr);

/*
 * Sends command for one device to perform a temperature conversion by index.
 * Returns false if a device is disconnected or if an operaiton failed.
 * Returns true otherwise.
 */
bool mgos_dallas_request_temperatures_by_index(Dallas *dt, int idx);

/*
 * Returns temperature raw value (12 bit integer of 1/128 degrees C)
 * or DEVICE_DISCONNECTED_RAW if an operaiton failed.
 */
int16_t mgos_dallas_get_temp(Dallas *dt, const uint8_t *addr);

/*
 * Returns temperature in degrees C * 100
 * or DEVICE_DISCONNECTED_C if an operaiton failed.
 */
int mgos_dallas_get_tempc(Dallas *dt, const uint8_t *addr);

/*
 * Returns temperature in degrees F * 100
 * or DEVICE_DISCONNECTED_F if an operaiton failed.
 */
int mgos_dallas_get_tempf(Dallas *dt, const uint8_t *addr);

/*
 * Returns temperature for device index in degrees C * 100 (slow)
 * or DEVICE_DISCONNECTED_C if an operaiton failed.
 */
int mgos_dallas_get_tempc_by_index(Dallas *dt, int idx);

/*
 * Returns temperature for device index in degrees F * 100 (slow)
 * or DEVICE_DISCONNECTED_F if an operaiton failed.
 */
int mgos_dallas_get_tempf_by_index(Dallas *dt, int idx);

/*
 * Returns true if the bus requires parasite power.
 * Returns always false if an operaiton failed.
 */
bool mgos_dallas_is_parasite_power_mode(Dallas *dt);

/*
 * Is a conversion complete on the wire?
 * Return always false if an operaiton failed.
 */
bool mgos_dallas_is_conversion_complete(Dallas *dt);

/*
 * Returns number of milliseconds to wait till conversion is complete (based on IC datasheet)
 * or 0 if an operaiton failed.
 */
int16_t mgos_dallas_millis_to_wait_for_conversion(Dallas *dt, int res);

#ifdef __cplusplus
}
#endif

