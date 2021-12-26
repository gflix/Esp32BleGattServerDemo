#include "BleServiceUuid.hpp"

namespace Esp32
{

BleServiceUuid::BleServiceUuid(
    Width width,
    uint32_t uuid,
    bool advertise):
    BleUuid(width, uuid),
    advertise(advertise)
{
}

BleServiceUuid::~BleServiceUuid()
{
}

} /* namespace Esp32 */
