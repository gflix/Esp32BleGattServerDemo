#ifndef MAIN_GENERICGATTCHARACTERISTIC_HPP_
#define MAIN_GENERICGATTCHARACTERISTIC_HPP_

#include <esp_gatts_api.h>
#include "BleUuid.hpp"

namespace Esp32
{

class GenericGattCharacteristic
{
public:
    GenericGattCharacteristic(
        const BleUuid& characteristicId,
        uint16_t length,
        uint16_t permission = ESP_GATT_PERM_READ,
        const char* description = nullptr);
    virtual ~GenericGattCharacteristic();

    const BleUuid& characteristicId(void) const;
    uint16_t length(void) const;
    uint16_t permission(void) const;
    const char* description(void) const;

    virtual void read(uint8_t* buffer, uint16_t* length);
    virtual void write(const uint8_t* buffer, uint16_t length);

    void setHandleIndex(int handleIndex);
    int handleIndex(void) const;

protected:

    BleUuid m_characteristicId;
    uint16_t m_length;
    uint16_t m_permission;
    const char* m_description;

    int m_handleIndex;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GENERICGATTCHARACTERISTIC_HPP_ */
