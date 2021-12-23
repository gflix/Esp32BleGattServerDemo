#ifndef MAIN_GENERICGATTCHARACTERISTIC_HPP_
#define MAIN_GENERICGATTCHARACTERISTIC_HPP_

#include <esp_gatts_api.h>

namespace Esp32
{

class GenericGattCharacteristic
{
public:
    GenericGattCharacteristic(
        uint16_t characteristicId,
        uint16_t permission = ESP_GATT_PERM_READ,
        const char* description = nullptr);
    virtual ~GenericGattCharacteristic();

    const uint16_t& characteristicId(void) const;
    const uint16_t& permission(void) const;
    const char* description(void) const;

    void setHandleIndex(int handleIndex);
    int handleIndex(void) const;

protected:

    uint16_t m_characteristicId;
    uint16_t m_permission;
    const char* m_description;

    int m_handleIndex;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GENERICGATTCHARACTERISTIC_HPP_ */
