#include <string.h>
#include <esp_log.h>
#include <stdexcept>
#include "GattsService.hpp"

#define LOG_TAG "GattsService"

namespace Esp32
{

const uint16_t GattsService::primaryServiceUuid = ESP_GATT_UUID_PRI_SERVICE;
const uint16_t GattsService::characterDeclarationUuid = ESP_GATT_UUID_CHAR_DECLARE;
const uint16_t GattsService::characterDescriptionUuid = ESP_GATT_UUID_CHAR_DESCRIPTION;
const uint8_t GattsService::characteristicPropertyRead = ESP_GATT_CHAR_PROP_BIT_READ;

GattsService::CharacteristicList::CharacteristicList(GenericGattCharacteristic* characteristic):
    characteristic(characteristic),
    next(nullptr)
{
}

GattsService::AttributeTable::AttributeTable(esp_gatts_attr_db_t* table, size_t length):
    table(table),
    length(length)
{
}

GattsService::GattsService(uint16_t serviceId):
    m_serviceId(serviceId),
    m_characteristics(nullptr)
{
}

GattsService::~GattsService()
{
}

uint16_t GattsService::serviceId(void) const
{
    return m_serviceId;
}

const GattsService::AttributeTable& GattsService::attributeTable(void)
{
    if (!m_attributeTable.table)
    {
        generateAttributeTable();
    }
    return m_attributeTable;
}

void GattsService::addCharacteristic(GenericGattCharacteristic* characteristic)
{
    if (!characteristic)
    {
        throw std::invalid_argument("null pointer exception");
    }

    auto characteristicListEntry = new CharacteristicList(characteristic);
    if (!m_characteristics)
    {
        m_characteristics = characteristicListEntry;
    }
    else
    {
        auto characteristicPointer = m_characteristics;
        for(; characteristicPointer->next; characteristicPointer = characteristicPointer->next)
        {
        }
        characteristicPointer->next = characteristicListEntry;
    }
}

void GattsService::pushHandles(const uint16_t* handles)
{
    if (!handles || !m_attributeTable.table)
    {
        throw std::invalid_argument("null pointer exception");
    }

    if (m_characteristicHandles)
    {
        throw std::invalid_argument("handles were already supplied");
    }

    m_characteristicHandles = (uint16_t*) malloc(m_attributeTable.length * sizeof(uint16_t));
    memcpy(m_characteristicHandles, handles, m_attributeTable.length * sizeof(uint16_t));
}

void GattsService::generateAttributeTable(void)
{
    if (m_attributeTable.table)
    {
        throw std::runtime_error("attribute table was already generated");
    }

    size_t requiredLength = 1;  // service declaration

    auto characteristicPointer = m_characteristics;
    while (characteristicPointer)
    {
        requiredLength +=
            2 +
            (characteristicPointer->characteristic->description() ? 1 : 0);

        characteristicPointer = characteristicPointer->next;
    }

    m_attributeTable.table = (esp_gatts_attr_db_t*) malloc(sizeof(esp_gatts_attr_db_t) * requiredLength);
    if (!m_attributeTable.table)
    {
        throw std::runtime_error("error allocating memory for the attribute table");
    }
    m_attributeTable.length = requiredLength;

    auto tablePointer = m_attributeTable.table;

    // put service declaration
    tablePointer->attr_control = { ESP_GATT_AUTO_RSP };
    tablePointer->att_desc = {
        ESP_UUID_LEN_16,
        (uint8_t *)&GattsService::primaryServiceUuid,
        ESP_GATT_PERM_READ,
        sizeof(m_serviceId),
        sizeof(m_serviceId),
        (uint8_t*) &m_serviceId
    };

    // put characteristic declarations
    characteristicPointer = m_characteristics;
    while (characteristicPointer)
    {
        ++tablePointer;
        tablePointer->attr_control = { ESP_GATT_AUTO_RSP };
        tablePointer->att_desc = {
            ESP_UUID_LEN_16,
            (uint8_t *)&GattsService::characterDeclarationUuid,
            ESP_GATT_PERM_READ,
            sizeof(characteristicPropertyRead),
            sizeof(characteristicPropertyRead),
            (uint8_t*) &characteristicPropertyRead
        };

        ++tablePointer;
        tablePointer->attr_control = { ESP_GATT_RSP_BY_APP };
        tablePointer->att_desc = {
            ESP_UUID_LEN_16,
            (uint8_t *)&characteristicPointer->characteristic->characteristicId(),
            ESP_GATT_PERM_READ,
            sizeof(m_dummyByte),
            sizeof(m_dummyByte),
            (uint8_t*) &m_dummyByte
        };

        auto description = characteristicPointer->characteristic->description();
        if (description)
        {
            ++tablePointer;
            tablePointer->attr_control = { ESP_GATT_AUTO_RSP };
            tablePointer->att_desc = {
                ESP_UUID_LEN_16,
                (uint8_t *)&GattsService::characterDescriptionUuid,
                ESP_GATT_PERM_READ,
                (uint16_t)strlen(description),
                (uint16_t)strlen(description),
                (uint8_t*) description
            };
        }

        characteristicPointer = characteristicPointer->next;
    }
}

} /* namespace Esp32 */
