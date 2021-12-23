#include "GenericGattCharacteristic.hpp"

namespace Esp32
{

GenericGattCharacteristic::GenericGattCharacteristic(
    uint16_t characteristicId,
    uint16_t permission,
    const char* description):
    m_characteristicId(characteristicId),
    m_permission(permission),
    m_description(description)
{
}

GenericGattCharacteristic::~GenericGattCharacteristic()
{
}

const uint16_t& GenericGattCharacteristic::characteristicId(void) const
{
    return m_characteristicId;
}

const uint16_t& GenericGattCharacteristic::permission(void) const
{
    return m_permission;
}

const char* GenericGattCharacteristic::description(void) const
{
    return m_description;
}

} /* namespace Esp32 */
