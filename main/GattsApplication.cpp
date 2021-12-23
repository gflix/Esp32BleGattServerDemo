#include <esp_log.h>
#include <stdexcept>
#include "GattsApplication.hpp"

#define LOG_TAG "GattsApplication"

#define CONFIGURATION_ADVERTISING_PENDING (1 << 0)
#define CONFIGURATION_SCAN_RESPONSE_PENDING (1 << 1)

namespace Esp32
{

esp_ble_adv_params_t GattsApplication::advertisingParameters = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

GattsApplication::GattsApplication(
    uint16_t applicationId,
    const char* deviceName,
    uint16_t appearance):
    m_applicationId(applicationId),
    m_deviceName(deviceName),
    m_appearance(appearance),
    m_configurationDone(0),
    m_interface(ESP_GATT_IF_NONE)
{
}

GattsApplication::~GattsApplication()
{
}

uint16_t GattsApplication::applicationId(void) const
{
    return m_applicationId;
}

void GattsApplication::gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
    switch (event)
    {
        // case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        //     handleGapEventRawAdvertisingDataSetComplete();
        //     break;
        // case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        //     handleGapEventRawScanResponseDataSetComplete();
        //     break;
        // case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //     handleGapEventAdvertisementStartComplete(param);
        //     break;
        // case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        //     handleGapEventUpdatedConnectionParameters(param);
        //     break;
        default:
            ESP_LOGI(LOG_TAG, "gapEventCallback(event=%d)", (int)event);
            throw std::runtime_error("not yet implemented");
    }
}

void GattsApplication::gattsEventCallback(
    esp_gatts_cb_event_t event,
    esp_gatt_if_t gatts_if,
    esp_ble_gatts_cb_param_t* param)
{
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status != ESP_GATT_OK)
        {
            char buffer[64];
            snprintf(
                buffer,
                sizeof(buffer) - 1,
                "error registering the application %04x, status %d",
                param->reg.app_id,
                param->reg.status);
            throw std::runtime_error(buffer);
        }

        m_interface = gatts_if;
    }

    switch (event)
    {
        // case ESP_GATTS_REG_EVT:
        //     handleGattsEventRegister(gatts_if);
        //     break;
        // case ESP_GATTS_READ_EVT:
        //     ESP_LOGI(LOG_TAG, "READ need_rsp=%d, handle=%d", (int)param->read.need_rsp, param->read.handle);
        //     if (param->read.need_rsp)
        //     {
        //         esp_gatt_rsp_t rsp;
        //     bzero(&rsp, sizeof(rsp));
        //     auto uptime = xTaskGetTickCount();
        //     rsp.attr_value.handle = param->read.handle;
        //     rsp.attr_value.len = sizeof(uptime);
        //     memcpy(rsp.attr_value.value, &uptime, sizeof(uptime));
        //     esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
        //                                 ESP_GATT_OK, &rsp);
        //     }
        //     break;
        // case ESP_GATTS_WRITE_EVT:
        //     handleGattsEventWrite(gatts_if, param);
        //     break;
        // case ESP_GATTS_MTU_EVT:
        //     handleGattsEventMtu(gatts_if, param);
        //     break;
        // case ESP_GATTS_START_EVT:
        //     ESP_LOGI(LOG_TAG, "SERVICE STARTED");
        //     break;
        // case ESP_GATTS_CONNECT_EVT:
        //     handleGattsEventConnect(gatts_if, param);
        //     break;
        // case ESP_GATTS_DISCONNECT_EVT:
        //     handleGattsEventDisconnect(gatts_if, param);
        //     break;
        // case ESP_GATTS_RESPONSE_EVT:
        //     ESP_LOGI(LOG_TAG, "RESPONSE COMPLETED");
        //     break;
        // case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        //     handleGattsEventCreateAttributeTable(gatts_if, param);
        //     break;
        default:
            ESP_LOGI(LOG_TAG, "gattsEventCallback(event=%d,gatts_if=%d)", (int)event, (int)gatts_if);
            throw std::runtime_error("not yet implemented");
    }
}

void GattsApplication::setConfigurationAdvertisingPendingFlag(void)
{
    m_configurationDone |= CONFIGURATION_ADVERTISING_PENDING;
}

void GattsApplication::setConfigurationAdvertisingDoneFlag(void)
{
    m_configurationDone &= (~CONFIGURATION_ADVERTISING_PENDING);
}

void GattsApplication::setConfigurationScanResponsePendingFlag(void)
{
    m_configurationDone |= CONFIGURATION_SCAN_RESPONSE_PENDING;
}

void GattsApplication::setConfigurationScanResponseDoneFlag(void)
{
    m_configurationDone &= (~CONFIGURATION_SCAN_RESPONSE_PENDING);
}

bool GattsApplication::configurationDone(void) const
{
    return m_configurationDone == 0;
}

} /* namespace Esp32 */
