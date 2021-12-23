#include <string.h>
#include <esp_log.h>
#include <stdexcept>
#include "ExampleGattsApplication.hpp"

#define LOG_TAG "ExampleGattsApplication"

#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define BLE_EXAMPLE_APPLICATION_ID (0x55)
#define BLE_EXAMPLE_ATTRIBUTE_HANDLES (3)

namespace Esp32
{

const uint16_t ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_A = 0x4000;
const uint16_t ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_B = 0x4100;
const uint16_t ExampleGattsApplication::GATTS_CHAR_UUID_TEST_A = 0x4010;
const uint16_t ExampleGattsApplication::GATTS_CHAR_UUID_TEST_B = 0x4110;

static uint32_t char_value[2] = { 0x43444546, 0x61626364 };

static const esp_gatts_attr_db_t gattAttributeTableA[BLE_EXAMPLE_ATTRIBUTE_HANDLES] =
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
            (uint8_t *)&ExampleGattsApplication::char_prop_read
        }
    },

    {
        { ESP_GATT_RSP_BY_APP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::GATTS_CHAR_UUID_TEST_A,
            ESP_GATT_PERM_READ,
            sizeof(char_value[0]),
            sizeof(char_value[0]),
            (uint8_t *)&char_value[0]
        }
    },
};

static const esp_gatts_attr_db_t gattAttributeTableB[BLE_EXAMPLE_ATTRIBUTE_HANDLES] =
{
    {
        { ESP_GATT_AUTO_RSP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::primary_service_uuid,
            ESP_GATT_PERM_READ,
            sizeof(uint16_t),
            sizeof(ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_B),
            (uint8_t *)&ExampleGattsApplication::GATTS_SERVICE_UUID_TEST_B
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
            (uint8_t *)&ExampleGattsApplication::char_prop_read
        }
    },

    {
        { ESP_GATT_RSP_BY_APP },
        {
            ESP_UUID_LEN_16,
            (uint8_t *)&ExampleGattsApplication::GATTS_CHAR_UUID_TEST_B,
            ESP_GATT_PERM_READ,
            sizeof(char_value[1]),
            sizeof(char_value[1]),
            (uint8_t *)&char_value[1]
        }
    },
};

static uint16_t gattHandleTableA[BLE_EXAMPLE_ATTRIBUTE_HANDLES];
static uint16_t gattHandleTableB[BLE_EXAMPLE_ATTRIBUTE_HANDLES];

ExampleGattsApplication::ExampleGattsApplication():
    GenericGattsApplication(BLE_EXAMPLE_APPLICATION_ID),
    m_startedServiceB(false)
{
}

ExampleGattsApplication::~ExampleGattsApplication()
{
}

void ExampleGattsApplication::registerAttibuteTable(esp_gatt_if_t gatts_if)
{
    if (esp_ble_gatts_create_attr_tab(gattAttributeTableA, gatts_if, BLE_EXAMPLE_ATTRIBUTE_HANDLES, 0) != ESP_OK)
    {
        throw std::runtime_error("error registering the GATT attribute table");
    }
}

void ExampleGattsApplication::startService(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    if (param->add_attr_tab.num_handle != BLE_EXAMPLE_ATTRIBUTE_HANDLES)
    {
        throw std::runtime_error("unexpected number of handles registered");
    }

    if (!m_startedServiceB)
    {
        memcpy(gattHandleTableA, param->add_attr_tab.handles, sizeof(gattHandleTableA));

        if (esp_ble_gatts_start_service(gattHandleTableA[0]) != ESP_OK)
        {
            throw std::runtime_error("error starting the GATT service");
        }
        ESP_LOGI(LOG_TAG, "startService(started A, trying to start B)");

        if (esp_ble_gatts_create_attr_tab(gattAttributeTableB, gatts_if, BLE_EXAMPLE_ATTRIBUTE_HANDLES, 1) != ESP_OK)
        {
            throw std::runtime_error("error registering the GATT attribute table");
        }
        ESP_LOGI(LOG_TAG, "startService(added table B)");
        m_startedServiceB = true;
    }
    else
    {
        memcpy(gattHandleTableB, param->add_attr_tab.handles, sizeof(gattHandleTableB));

        if (esp_ble_gatts_start_service(gattHandleTableB[0]) != ESP_OK)
        {
            throw std::runtime_error("error starting the GATT service");
        }
        ESP_LOGI(LOG_TAG, "startService(started B)");
    }
}

void ExampleGattsApplication::writeAttribute(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGI(LOG_TAG, "writeAttribute(handle=%d, len=%d)", param->write.handle, param->write.len);
}

} /* namespace Esp32 */
