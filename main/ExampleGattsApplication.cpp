#include <string.h>
#include <esp_log.h>
#include <stdexcept>
#include "ExampleGattsApplication.hpp"

#define LOG_TAG "ExampleGattsApplication"

#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define BLE_EXAMPLE_APPLICATION_ID (0x55)
#define BLE_EXAMPLE_ATTRIBUTE_HANDLES (7)

namespace Esp32
{

const uint16_t ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_A = 0x181c;
const uint16_t ExampleGattsApplication::GATTS_CHAR_UUID_TEST_A = 0xFF01;
const uint16_t ExampleGattsApplication::GATTS_CHAR_UUID_TEST_B = 0xFF02;

static uint32_t char_value[2] = { 0x43444546, 0x61626364 };
static uint16_t heart_measurement_ccc = 0;
static char description[] = "FooBar";

static const esp_gatts_attr_db_t gattAttributeTable[BLE_EXAMPLE_ATTRIBUTE_HANDLES] =
{
    {
        { ESP_GATT_AUTO_RSP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::primary_service_uuid,
            ESP_GATT_PERM_READ,
            sizeof(uint16_t),
            sizeof(ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_A),
            (uint8_t *)&ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_A
        }
    },

    {
        { ESP_GATT_AUTO_RSP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::character_declaration_uuid,
            ESP_GATT_PERM_READ,
            CHAR_DECLARATION_SIZE,
            CHAR_DECLARATION_SIZE,
            (uint8_t *)&ExampleGattsApplication::char_prop_read_write_notify
        }
    },

    {
        { ESP_GATT_RSP_BY_APP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::GATTS_CHAR_UUID_TEST_A,
            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            sizeof(char_value[0]),
            sizeof(char_value[0]),
            (uint8_t *)&char_value[0]
        }
    },

    {
        { ESP_GATT_AUTO_RSP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::character_declaration_uuid,
            ESP_GATT_PERM_READ,
            CHAR_DECLARATION_SIZE,
            CHAR_DECLARATION_SIZE,
            (uint8_t *)&ExampleGattsApplication::char_prop_read_write_notify
        }
    },

    {
        { ESP_GATT_RSP_BY_APP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::GATTS_CHAR_UUID_TEST_B,
            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            sizeof(char_value[1]),
            sizeof(char_value[1]),
            (uint8_t *)&char_value[1]
        }
    },

    {
        { ESP_GATT_AUTO_RSP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::character_client_config_uuid,
            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            sizeof(uint16_t),
            sizeof(heart_measurement_ccc),
            (uint8_t *)&heart_measurement_ccc
        }
    },

    {
        { ESP_GATT_AUTO_RSP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::character_description_uuid,
            ESP_GATT_PERM_READ,
            sizeof(description) - 1,
            sizeof(description) - 1,
            (uint8_t *)description
        }
    },
};

static uint16_t gattHandleTable[BLE_EXAMPLE_ATTRIBUTE_HANDLES];

ExampleGattsApplication::ExampleGattsApplication():
    GenericGattsApplication(BLE_EXAMPLE_APPLICATION_ID)
{
}

ExampleGattsApplication::~ExampleGattsApplication()
{
}

void ExampleGattsApplication::registerAttibuteTable(esp_gatt_if_t gatts_if)
{
    if (esp_ble_gatts_create_attr_tab(gattAttributeTable, gatts_if, BLE_EXAMPLE_ATTRIBUTE_HANDLES, 0) != ESP_OK)
    {
        throw std::runtime_error("error registering the GATT attribute table");
    }
}

void ExampleGattsApplication::startService(esp_ble_gatts_cb_param_t* param)
{
    if (param->add_attr_tab.num_handle != BLE_EXAMPLE_ATTRIBUTE_HANDLES)
    {
        throw std::runtime_error("unexpected number of handles registered");
    }

    memcpy(gattHandleTable, param->add_attr_tab.handles, sizeof(gattHandleTable));

    if (esp_ble_gatts_start_service(gattHandleTable[0]) != ESP_OK)
    {
        throw std::runtime_error("error starting the GATT service");
    }
}

void ExampleGattsApplication::writeAttribute(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGI(LOG_TAG, "writeAttribute(handle=%d, len=%d, char_value[0])", param->write.handle, param->write.len);

    if (param->write.handle == gattHandleTable[2] && param->write.len == sizeof(char_value[0]))
    {
        memcpy(&char_value[0], param->write.value, param->write.len);
        ESP_LOGI(LOG_TAG, "writeAttribute(new value[0]=%08x)", char_value[0]);
    }
    if (param->write.handle == gattHandleTable[4] && param->write.len == sizeof(char_value[1]))
    {
        memcpy(&char_value[1], param->write.value, param->write.len);
        ESP_LOGI(LOG_TAG, "writeAttribute(new value[1]=%08x)", char_value[1]);
    }
}

} /* namespace Esp32 */
