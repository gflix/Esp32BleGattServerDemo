#ifndef MAIN_UINT16GATTCHARACTERISTIC_HPP_
#define MAIN_UINT16GATTCHARACTERISTIC_HPP_

#include "GenericGattCharacteristic.hpp"

namespace Esp32
{

class UInt16GattCharacteristic: public GenericGattCharacteristic
{
public:
    UInt16GattCharacteristic(
        const BleUuid& characteristicId,
        uint16_t permission = ESP_GATT_PERM_READ,
        const char* description = nullptr,
        uint16_t defaultValue = 0);
    virtual ~UInt16GattCharacteristic();

    void read(uint8_t* buffer, uint16_t* length) override;
    void write(const uint8_t* buffer, uint16_t length) override;

protected:

    uint16_t m_value;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_UINT16GATTCHARACTERISTIC_HPP_ */
