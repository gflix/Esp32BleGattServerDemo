#ifndef MAIN_GATTSSERVICE_HPP_
#define MAIN_GATTSSERVICE_HPP_

#include <esp_gatts_api.h>
#include "GenericGattCharacteristic.hpp"

namespace Esp32
{

class GattsService
{
public:
    struct CharacteristicList
    {
        CharacteristicList(GenericGattCharacteristic* characteristic);

        GenericGattCharacteristic* characteristic;
        CharacteristicList* next;
    };

    struct AttributeTable
    {
        AttributeTable(esp_gatts_attr_db_t* table = nullptr, size_t length = 0);

        esp_gatts_attr_db_t* table;
        size_t length;
    };

    GattsService(uint16_t serviceId);
    virtual ~GattsService();

    uint16_t serviceId(void) const;
    const AttributeTable& attributeTable(void);

    void addCharacteristic(GenericGattCharacteristic* characteristic);
    void pushHandles(const uint16_t* handles);

    static const uint16_t primaryServiceUuid;
    static const uint16_t characterDeclarationUuid;
    static const uint8_t characteristicPropertyRead;

protected:

    uint16_t m_serviceId;

    CharacteristicList* m_characteristics;
    AttributeTable m_attributeTable;
    uint16_t* m_characteristicHandles;

    uint8_t m_dummyByte;

    void generateAttributeTable(void);

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GATTSSERVICE_HPP_ */
