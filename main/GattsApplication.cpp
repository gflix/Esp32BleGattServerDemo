#include <string.h>
#include <esp_log.h>
#include <stdexcept>
#include "GattsApplication.hpp"

#define LOG_TAG "GattsApplication"

#define ADVERTISEMENT_LENGTH_MAX (31)

#define CONFIGURATION_ADVERTISEMENT_PENDING (1 << 0)
#define CONFIGURATION_SCAN_RESPONSE_PENDING (1 << 1)

namespace Esp32
{

const uint8_t GattsApplication::advertisementFlags[3] = { 0x02, 0x01, 0x06 };

esp_ble_adv_params_t GattsApplication::advertisementParameters = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

GattsApplication::AdvertisementData::AdvertisementData(uint8_t* payload, size_t length):
    payload(payload),
    length(length)
{
}

void GattsApplication::AdvertisementData::dump(void) const
{
    char buffer[128];
    auto remainingSize = sizeof(buffer);
    int writtenBytes;
    auto bufferPointer = buffer;

    writtenBytes = snprintf(bufferPointer, remainingSize, "AD[");

    if (!payload)
    {
        remainingSize -= writtenBytes;
        bufferPointer += writtenBytes;
        writtenBytes = snprintf(bufferPointer, remainingSize, "nullptr");
    }
    else
    {
        auto payloadPointer = payload;
        for (auto i = length; i > 0; --i, ++payloadPointer)
        {
            remainingSize -= writtenBytes;
            bufferPointer += writtenBytes;
            writtenBytes = snprintf(bufferPointer, remainingSize, "%02x ", (int)*payloadPointer);
        }
    }
    remainingSize -= writtenBytes;
    bufferPointer += writtenBytes;
    writtenBytes = snprintf(bufferPointer, remainingSize, "]");

    ESP_LOGI(LOG_TAG, "dump(%s)", buffer);
}

GattsApplication::GattsApplication(
    uint16_t applicationId,
    const char* shortDeviceName,
    const char* fullDeviceName,
    uint16_t appearance):
    m_applicationId(applicationId),
    m_shortDeviceName(shortDeviceName),
    m_fullDeviceName(fullDeviceName),
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
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            handleGapEventAdvertisementDataSetComplete();
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            handleGapEventScanResponseDataSetComplete();
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            handleGapEventAdvertisementStartComplete(param);
            break;
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
        case ESP_GATTS_REG_EVT:
            handleGattsEventRegister(gatts_if);
            break;
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

void GattsApplication::handleGapEventAdvertisementDataSetComplete(void)
{
    setConfigurationAdvertisementDoneFlag();
    if (configurationDone())
    {
        esp_ble_gap_start_advertising(&advertisementParameters);
    }
}

void GattsApplication::handleGapEventScanResponseDataSetComplete(void)
{
    setConfigurationScanResponseDoneFlag();
    if (configurationDone())
    {
        esp_ble_gap_start_advertising(&advertisementParameters);
    }
}

void GattsApplication::handleGapEventAdvertisementStartComplete(esp_ble_gap_cb_param_t* param)
{
    if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS)
    {
        ESP_LOGI(LOG_TAG, "started advertising");
    }
    else
    {
        ESP_LOGE(LOG_TAG, "error starting advertising");
    }
}

void GattsApplication::handleGattsEventRegister(esp_gatt_if_t gatts_if)
{
    auto deviceName = m_fullDeviceName;
    if (!deviceName)
    {
        deviceName = m_shortDeviceName;
    }
    if (!deviceName)
    {
        throw std::runtime_error("no device name available");
    }
    if (esp_ble_gap_set_device_name(deviceName) != ESP_OK)
    {
        throw std::runtime_error("error setting the device name");
    }

    generateRawAdvertisementData();
    m_rawAdvertisementData.dump();
    generateRawScanResponseData();
    m_rawScanResponseData.dump();

    if (esp_ble_gap_config_adv_data_raw(m_rawAdvertisementData.payload, m_rawAdvertisementData.length) != ESP_OK)
    {
        throw std::runtime_error("error setting raw advertising data");
    }
    setConfigurationAdvertisementPendingFlag();

    if (esp_ble_gap_config_scan_rsp_data_raw(m_rawScanResponseData.payload, m_rawScanResponseData.length) != ESP_OK)
    {
        throw std::runtime_error("error setting raw scan response data");
    }
    setConfigurationScanResponsePendingFlag();

    // registerAttibuteTable(gatts_if);
}

void GattsApplication::generateRawAdvertisementData(void)
{
    if (m_rawAdvertisementData.payload)
    {
        throw std::runtime_error("advertisement data was already generated");
    }

    size_t requiredLength = sizeof(advertisementFlags);

    if (!m_shortDeviceName)
    {
        throw std::runtime_error("no short device name given");
    }
    requiredLength += 2 + strlen(m_shortDeviceName);

    if (requiredLength > ADVERTISEMENT_LENGTH_MAX)
    {
        char buffer[64];
        snprintf(
            buffer,
            sizeof(buffer) - 1,
            "advertisement to long (now %d bytes, max accepted is %d)",
            requiredLength,
            ADVERTISEMENT_LENGTH_MAX);
        throw std::runtime_error(buffer);
    }

    m_rawAdvertisementData.payload = (uint8_t*) malloc(requiredLength * sizeof(uint8_t));
    if (!m_rawAdvertisementData.payload)
    {
        throw std::runtime_error("error allocating memory for the advertisment data");
    }
    m_rawAdvertisementData.length = requiredLength;

    auto payloadPointer = m_rawAdvertisementData.payload;
    memcpy(payloadPointer, advertisementFlags, sizeof(advertisementFlags));
    payloadPointer += sizeof(advertisementFlags);

    *payloadPointer = strlen(m_shortDeviceName) + 1;
    ++payloadPointer;
    *payloadPointer = 0x09;
    ++payloadPointer;
    memcpy(payloadPointer, m_shortDeviceName, strlen(m_shortDeviceName));
    payloadPointer += strlen(m_shortDeviceName);
}

void GattsApplication::generateRawScanResponseData(void)
{
    if (m_rawScanResponseData.payload)
    {
        throw std::runtime_error("scan response data was already generated");
    }

    size_t requiredLength = sizeof(advertisementFlags);

    requiredLength += 4;  // appearance

    if (requiredLength > ADVERTISEMENT_LENGTH_MAX)
    {
        char buffer[64];
        snprintf(
            buffer,
            sizeof(buffer) - 1,
            "scan response to long (now %d bytes, max accepted is %d)",
            requiredLength,
            ADVERTISEMENT_LENGTH_MAX);
        throw std::runtime_error(buffer);
    }

    m_rawScanResponseData.payload = (uint8_t*) malloc(requiredLength * sizeof(uint8_t));
    if (!m_rawScanResponseData.payload)
    {
        throw std::runtime_error("error allocating memory for the advertisment data");
    }
    m_rawScanResponseData.length = requiredLength;

    auto payloadPointer = m_rawScanResponseData.payload;
    memcpy(payloadPointer, advertisementFlags, sizeof(advertisementFlags));
    payloadPointer += sizeof(advertisementFlags);

    *payloadPointer = 3;
    ++payloadPointer;
    *payloadPointer = 0x19;
    ++payloadPointer;
    memcpy(payloadPointer, &m_appearance, sizeof(m_appearance));
    payloadPointer += sizeof(m_appearance);
}

void GattsApplication::setConfigurationAdvertisementPendingFlag(void)
{
    m_configurationDone |= CONFIGURATION_ADVERTISEMENT_PENDING;
}

void GattsApplication::setConfigurationAdvertisementDoneFlag(void)
{
    m_configurationDone &= (~CONFIGURATION_ADVERTISEMENT_PENDING);
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
