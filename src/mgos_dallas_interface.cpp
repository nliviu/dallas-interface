#include "mgos_dallas_interface.h"
#include <math.h>

#ifndef NULL
#define NULL 0
#endif

void mgos_dallas_close(Dallas* dt)
{
    if (dt != NULL) {
        delete dt;
        dt = NULL;
    }
}

void mgos_dallas_begin(Dallas* dt)
{
    if (NULL != dt) {
        dt->begin();
    }
}

int mgos_dallas_get_device_count(Dallas* dt)
{
    return (NULL == dt) ? 0
        : dt->getDeviceCount();
}

bool mgos_dallas_valid_address(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? false
        : dt->validAddress((uint8_t* ) addr);
}

bool mgos_dallas_valid_family(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? false
        : dt->validFamily((uint8_t* ) addr);
}

bool mgos_dallas_get_address(Dallas* dt, uint8_t* addr, int idx)
{
    return (NULL == dt) ? false
        : dt->getAddress((uint8_t* ) addr, idx);
}

bool mgos_dallas_is_connected(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? false
        : dt->isConnected((uint8_t* ) addr);
}

bool mgos_dallas_is_connected_sp(Dallas* dt, const uint8_t* addr, uint8_t* sp)
{
    return (NULL == dt) ? false
        : dt->isConnected((uint8_t* ) addr, (uint8_t* ) sp);
}

bool mgos_dallas_read_scratch_pad(Dallas* dt, const uint8_t* addr, uint8_t* sp)
{
    return (NULL == dt) ? false
        : dt->readScratchPad((uint8_t* ) addr, (uint8_t* ) sp);
}

void mgos_dallas_write_scratch_pad(Dallas* dt, const uint8_t* addr, const uint8_t* sp)
{
    if (NULL != dt) {
        dt->writeScratchPad((uint8_t* ) addr, (uint8_t* ) sp);
    }
}

bool mgos_dallas_read_power_supply(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? false
        : dt->readPowerSupply((uint8_t* ) addr);
}

int mgos_dallas_get_global_resolution(Dallas* dt)
{
    return (NULL == dt) ? 0
        : dt->getResolution();
}

void mgos_dallas_set_global_resolution(Dallas* dt, int res)
{
    if (NULL != dt) {
        dt->setResolution(res);
    }
}

int mgos_dallas_get_resolution(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? 0
        : dt->getResolution((uint8_t* ) addr);
}

bool mgos_dallas_set_resolution(Dallas* dt, const uint8_t* addr, int res, bool skip_global_calc)
{
    return (NULL == dt) ? false
        : dt->setResolution((uint8_t* ) addr, res, skip_global_calc);
}

void mgos_dallas_set_wait_for_conversion(Dallas* dt, bool f)
{
    if (NULL != dt) {
        dt->setWaitForConversion(f);
    }
}

bool mgos_dallas_get_wait_for_conversion(Dallas* dt)
{
    return (NULL == dt) ? false
        : dt->getWaitForConversion();
}

void mgos_dallas_set_check_for_conversion(Dallas* dt, bool f)
{
    if (NULL != dt) {
        dt->setCheckForConversion(f);
    }
}

bool mgos_dallas_get_check_for_conversion(Dallas* dt)
{
    return (NULL == dt) ? false
        : dt->getCheckForConversion();
}

void mgos_dallas_request_temperatures(Dallas* dt)
{
    if (NULL != dt) {
        dt->requestTemperatures();
    }
}

bool mgos_dallas_request_temperatures_by_address(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? false
        : dt->requestTemperaturesByAddress((uint8_t* ) addr);
}

bool mgos_dallas_request_temperatures_by_index(Dallas* dt, int idx)
{
    return (NULL == dt) ? false
        : dt->requestTemperaturesByIndex(idx);
}

int16_t mgos_dallas_get_temp(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? DEVICE_DISCONNECTED_RAW
        : dt->getTemp((uint8_t* ) addr);
}

int mgos_dallas_get_tempc(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? DEVICE_DISCONNECTED_C
        : round(dt->getTempC((uint8_t* ) addr)*  100.0);//(int) (0.5 + dt->getTempC((uint8_t* ) addr)*  100.0);
}

int mgos_dallas_get_tempf(Dallas* dt, const uint8_t* addr)
{
    return (NULL == dt) ? DEVICE_DISCONNECTED_F
        : (int) (0.5 + dt->getTempF((uint8_t* ) addr)*  100.0);
}

int mgos_dallas_get_tempc_by_index(Dallas* dt, int idx)
{
    return (NULL == dt) ? DEVICE_DISCONNECTED_C
        : (int) (0.5 + dt->getTempCByIndex(idx)*  100.0);
}

int mgos_dallas_get_tempf_by_index(Dallas* dt, int idx)
{
    return (NULL == dt) ? DEVICE_DISCONNECTED_F
        : (int) (0.5 + dt->getTempFByIndex(idx)*  100.0);
}

bool mgos_dallas_is_parasite_power_mode(Dallas* dt)
{
    return (NULL == dt) ? false
        : dt->isParasitePowerMode();
}

bool mgos_dallas_is_conversion_complete(Dallas* dt)
{
    return (NULL == dt) ? false
        : dt->isConversionComplete();
}

int16_t mgos_dallas_millis_to_wait_for_conversion(Dallas* dt, int res)
{
    return (NULL == dt) ? 0
        : dt->millisToWaitForConversion(res);
}
