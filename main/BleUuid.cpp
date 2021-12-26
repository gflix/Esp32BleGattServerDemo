#include "BleUuid.hpp"

namespace Esp32
{

BleUuid::BleUuid(
    Width width,
    uint32_t uuid):
    width(width),
    uuid16((uint16_t) uuid),
    uuid32(uuid)
{
}

BleUuid::~BleUuid()
{
}

} /* namespace Esp32 */
