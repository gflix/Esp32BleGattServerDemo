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
const uint8_t GattsService::characteristicPropertyReadWrite =
    ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
const uint8_t GattsService::characteristicPropertyWrite = ESP_GATT_CHAR_PROP_BIT_WRITE;

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

GattsService::GattsService(const BleServiceUuid& serviceId):
    m_serviceId(serviceId),
    m_characteristics(nullptr),
    m_characteristicHandles(nullptr)
{
}

GattsService::~GattsService()
{
}

const BleServiceUuid& GattsService::serviceId(void) const
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

void GattsService::readCharacteristic(uint16_t handle, uint8_t* buffer, uint16_t* length)
{
    getCharacteristicForHandle(handle)->read(buffer, length);
}

void GattsService::writeCharacteristic(uint16_t handle, const uint8_t* buffer, uint16_t length)
{
    getCharacteristicForHandle(handle)->write(buffer, length);
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

bool GattsService::hasHandle(uint16_t handle)
{
    if (!m_characteristicHandles)
    {
        return false;
    }

    for (auto i = 0; i < m_attributeTable.length; ++i)
    {
        if (m_characteristicHandles[i] == handle)
        {
            return true;
        }
    }
    return false;
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
        0,
        0,
        nullptr
    };
    switch (m_serviceId.width)
    {
        case BleUuid::Width::UUID_16:
            tablePointer->att_desc.length = sizeof(m_serviceId.uuid16);
            tablePointer->att_desc.max_length = sizeof(m_serviceId.uuid16);
            tablePointer->att_desc.value = (uint8_t*) &m_serviceId.uuid16;
            break;
        case BleUuid::Width::UUID_32:
            tablePointer->att_desc.length = sizeof(m_serviceId.uuid32);
            tablePointer->att_desc.max_length = sizeof(m_serviceId.uuid32);
            tablePointer->att_desc.value = (uint8_t*) &m_serviceId.uuid32;
            break;
        default:
            throw std::runtime_error("UUID width is not supported");
    }

    // put characteristic declarations
    characteristicPointer = m_characteristics;
    while (characteristicPointer)
    {
        auto permission = characteristicPointer->characteristic->permission();
        auto characteristicProperty = permissionBitmaskToCharacteristicProperty(permission);

        ++tablePointer;
        tablePointer->attr_control = { ESP_GATT_AUTO_RSP };
        tablePointer->att_desc = {
            ESP_UUID_LEN_16,
            (uint8_t *)&GattsService::characterDeclarationUuid,
            ESP_GATT_PERM_READ,
            sizeof(characteristicProperty),
            sizeof(characteristicProperty),
            (uint8_t*) characteristicProperty
        };

        ++tablePointer;
        tablePointer->attr_control = { ESP_GATT_RSP_BY_APP };
        auto& characteristicId = characteristicPointer->characteristic->characteristicId();

        tablePointer->att_desc = {
            0,
            nullptr,
            permission,
            characteristicPointer->characteristic->length(),
            characteristicPointer->characteristic->length(),
            &m_dummyByte
        };

        switch (characteristicId.width)
        {
            case BleUuid::Width::UUID_16:
                tablePointer->att_desc.uuid_length = ESP_UUID_LEN_16;
                tablePointer->att_desc.uuid_p =
                    (uint8_t*) &characteristicPointer->characteristic->characteristicId().uuid16; // TODO try reference
                break;
            case BleUuid::Width::UUID_32:
                tablePointer->att_desc.uuid_length = ESP_UUID_LEN_32;
                tablePointer->att_desc.uuid_p =
                    (uint8_t*) &characteristicPointer->characteristic->characteristicId().uuid32;
                break;
            default:
                throw std::runtime_error("UUID width is not supported");
        }
        characteristicPointer->characteristic->setHandleIndex(tablePointer - m_attributeTable.table);

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

GenericGattCharacteristic* GattsService::getCharacteristicForHandle(uint16_t handle)
{
    if (!m_characteristicHandles)
    {
        throw std::runtime_error("no handles initialized yet");
    }

    for (auto i = 0; i < m_attributeTable.length; ++i)
    {
        if (m_characteristicHandles[i] == handle)
        {
            auto characteristicPointer = m_characteristics;

            while (characteristicPointer)
            {
                if (characteristicPointer->characteristic->handleIndex() == i)
                {
                    return characteristicPointer->characteristic;
                }

                characteristicPointer = characteristicPointer->next;
            }
            throw std::runtime_error("requested characteristic not found");
        }
    }
    throw std::runtime_error("requested handle not found");
}

const uint8_t* GattsService::permissionBitmaskToCharacteristicProperty(uint8_t permission)
{
    switch (permission)
    {
        case ESP_GATT_PERM_READ:
            return &characteristicPropertyRead;
        case ESP_GATT_PERM_WRITE:
            return &characteristicPropertyWrite;
        case ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE:
            return &characteristicPropertyReadWrite;
        default:
            break;
    }
    throw std::invalid_argument("unsupported permission");
}

} /* namespace Esp32 */
