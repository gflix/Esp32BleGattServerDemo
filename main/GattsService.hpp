#ifndef MAIN_GATTSSERVICE_HPP_
#define MAIN_GATTSSERVICE_HPP_

#include <esp_gatts_api.h>
#include "BleServiceUuid.hpp"
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

    GattsService(const BleServiceUuid& serviceId);
    virtual ~GattsService();

    const BleServiceUuid& serviceId(void) const;
    const AttributeTable& attributeTable(void);

    void addCharacteristic(GenericGattCharacteristic* characteristic);
    void readCharacteristic(uint16_t handle, uint8_t* buffer, uint16_t* length);
    virtual void writeCharacteristic(uint16_t handle, const uint8_t* buffer, uint16_t length);
    void pushHandles(const uint16_t* handles);
    bool hasHandle(uint16_t handle);

    static const uint16_t primaryServiceUuid;
    static const uint16_t characterDeclarationUuid;
    static const uint16_t characterDescriptionUuid;
    static const uint8_t characteristicPropertyRead;
    static const uint8_t characteristicPropertyReadWrite;
    static const uint8_t characteristicPropertyWrite;

protected:

    BleServiceUuid m_serviceId;

    CharacteristicList* m_characteristics;
    AttributeTable m_attributeTable;
    uint16_t* m_characteristicHandles;

    uint8_t m_dummyByte;

    void generateAttributeTable(void);
    GenericGattCharacteristic* getCharacteristicForHandle(uint16_t handle);
    const uint8_t* permissionBitmaskToCharacteristicProperty(uint8_t permission);

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GATTSSERVICE_HPP_ */
