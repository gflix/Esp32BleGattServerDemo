#ifndef MAIN_GENERICGATTCHARACTERISTIC_HPP_
#define MAIN_GENERICGATTCHARACTERISTIC_HPP_

#include <esp_gatts_api.h>

namespace Esp32
{

class GenericGattCharacteristic
{
public:
    GenericGattCharacteristic(uint16_t characteristicId, uint16_t permission = ESP_GATT_PERM_READ);
    virtual ~GenericGattCharacteristic();

    const uint16_t& characteristicId(void) const;
    const uint16_t& permission(void) const;

protected:

    uint16_t m_characteristicId;
    uint16_t m_permission;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GENERICGATTCHARACTERISTIC_HPP_ */
