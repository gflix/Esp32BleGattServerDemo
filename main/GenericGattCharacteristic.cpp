#include "GenericGattCharacteristic.hpp"

namespace Esp32
{

GenericGattCharacteristic::GenericGattCharacteristic(uint16_t characteristicId, uint16_t permission):
    m_characteristicId(characteristicId),
    m_permission(permission)
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


} /* namespace Esp32 */
