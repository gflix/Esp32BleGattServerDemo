#include <stdexcept>
#include "GenericGattCharacteristic.hpp"

namespace Esp32
{

GenericGattCharacteristic::GenericGattCharacteristic(
    const BleUuid& characteristicId,
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

const BleUuid& GenericGattCharacteristic::characteristicId(void) const
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

void GenericGattCharacteristic::read(uint8_t* buffer, uint16_t* length)
{
    throw std::runtime_error("reading from characteristic not supported");
}

void GenericGattCharacteristic::write(const uint8_t* buffer, uint16_t length)
{
    throw std::runtime_error("writing to characteristic not supported");
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
