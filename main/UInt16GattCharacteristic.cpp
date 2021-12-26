#include <string.h>
#include <stdexcept>
#include "UInt16GattCharacteristic.hpp"

namespace Esp32
{

UInt16GattCharacteristic::UInt16GattCharacteristic(
    const BleUuid& characteristicId,
    uint16_t permission,
    const char* description,
    uint16_t defaultValue):
    GenericGattCharacteristic(characteristicId, sizeof(uint16_t), permission, description),
    m_value(defaultValue)
{
}

UInt16GattCharacteristic::~UInt16GattCharacteristic()
{
}

void UInt16GattCharacteristic::read(uint8_t* buffer, uint16_t* length)
{
    *length = m_length;
    memcpy(buffer, &m_value, m_length);
}

void UInt16GattCharacteristic::write(const uint8_t* buffer, uint16_t length)
{
    if (length != m_length)
    {
        throw std::invalid_argument("invalid length");
    }

    memcpy(&m_value, buffer, length);
}

} /* namespace Esp32 */
