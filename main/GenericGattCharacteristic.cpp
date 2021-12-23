#include <stdexcept>
#include "GenericGattCharacteristic.hpp"

namespace Esp32
{

GenericGattCharacteristic::GenericGattCharacteristic(
    uint16_t characteristicId,
    uint16_t length,
    uint16_t permission,
    const char* description):
    m_characteristicId(characteristicId),
    m_length(length),
    m_permission(permission),
    m_description(description),
    m_handleIndex(-1)
{
}

GenericGattCharacteristic::~GenericGattCharacteristic()
{
}

const uint16_t& GenericGattCharacteristic::characteristicId(void) const
{
    return m_characteristicId;
}

uint16_t GenericGattCharacteristic::length(void) const
{
    return m_length;
}

uint16_t GenericGattCharacteristic::permission(void) const
{
    return m_permission;
}

const char* GenericGattCharacteristic::description(void) const
{
    return m_description;
}

void GenericGattCharacteristic::setHandleIndex(int handleIndex)
{
    m_handleIndex = handleIndex;
}

int GenericGattCharacteristic::handleIndex(void) const
{
    return m_handleIndex;
}

} /* namespace Esp32 */
