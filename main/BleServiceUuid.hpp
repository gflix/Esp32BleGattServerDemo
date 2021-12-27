#ifndef MAIN_BLESERVICEUUID_HPP_
#define MAIN_BLESERVICEUUID_HPP_

#include "BleUuid.hpp"

namespace Esp32
{

struct BleServiceUuid: public BleUuid
{
    BleServiceUuid(
        Width width,
        uint32_t uuid,
        bool advertise = true);
    virtual ~BleServiceUuid();

    bool advertise;
};

} /* namespace Esp32 */

#endif /* MAIN_BLESERVICEUUID_HPP_ */
