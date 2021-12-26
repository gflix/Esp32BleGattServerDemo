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

    writtenBytes = snprintf(bufferPointer, remainingSize, "AD[%2d: ", length);

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

GattsApplication::ServiceList::ServiceList(GattsService* service):
    service(service),
    next(nullptr)
{
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
    m_services(nullptr),
    m_nextServiceForRegistration(nullptr),
    m_nextServiceRegistrationNumber(0),
    m_configurationDone(0),
    m_interface(ESP_GATT_IF_NONE),
    m_dummyValue(0)
{
}

GattsApplication::~GattsApplication()
{
}

uint16_t GattsApplication::applicationId(void) const
{
    return m_applicationId;
}

void GattsApplication::addService(GattsService* service)
{
    if (!service)
    {
        throw std::invalid_argument("null pointer exception");
    }

    auto serviceListEntry = new ServiceList(service);
    if (!m_services)
    {
        m_services = serviceListEntry;
    }
    else
    {
        auto servicePointer = m_services;
        for(; servicePointer->next; servicePointer = servicePointer->next)
        {
        }
        servicePointer->next = serviceListEntry;
    }
}

int GattsApplication::numberOfAdvertisedServices(BleUuid::Width width) const
{
    int counter = 0;
    auto servicePointer = m_services;
    while (servicePointer)
    {
        auto& serviceId = servicePointer->service->serviceId();
        if (serviceId.width == width && serviceId.advertise)
        {
            ++counter;
        }
        servicePointer = servicePointer->next;
    }

    return counter;
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
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            handleGapEventUpdatedConnectionParameters(param);
            break;
        default:
            ESP_LOGW(LOG_TAG, "gapEventCallback(event=%d)", (int)event);
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
        case ESP_GATTS_READ_EVT:
            handleGattsEventRead(gatts_if, param);
            break;
        case ESP_GATTS_WRITE_EVT:
            handleGattsEventWrite(gatts_if, param);
            break;
        case ESP_GATTS_MTU_EVT:
            handleGattsEventMtu(gatts_if, param);
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGD(LOG_TAG, "GATTS event: service started");
            m_nextServiceForRegistration = m_nextServiceForRegistration->next;
            ++m_nextServiceRegistrationNumber;

            registerNextService(gatts_if);

            break;
        case ESP_GATTS_CONNECT_EVT:
            handleGattsEventConnect(gatts_if, param);
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            handleGattsEventDisconnect(gatts_if, param);
            break;
        case ESP_GATTS_RESPONSE_EVT:
            ESP_LOGD(LOG_TAG, "GATTS event: response completed");
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            handleGattsEventCreateAttributeTable(gatts_if, param);
            break;
        default:
            ESP_LOGW(LOG_TAG, "gattsEventCallback(event=%d,gatts_if=%d)", (int)event, (int)gatts_if);
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

void GattsApplication::handleGapEventUpdatedConnectionParameters(esp_ble_gap_cb_param_t* param)
{
    ESP_LOGD(
        LOG_TAG,
        "UPDATED CONN PARAMS, status=%d, min_int=%d, max_int=%d, conn_int=%d, latency=%d, timeout=%d",
        param->update_conn_params.status,
        (param->update_conn_params.min_int * 125) / 100,
        (param->update_conn_params.max_int * 125) / 100,
        param->update_conn_params.conn_int,
        param->update_conn_params.latency,
        param->update_conn_params.timeout * 10);
}

void GattsApplication::handleGattsEventConnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGI(
        LOG_TAG,
        "CONNECT, conn_id=%d, address=%02x:%02x:%02x:%02x:%02x:%02x",
        param->connect.conn_id,
        param->connect.remote_bda[0],
        param->connect.remote_bda[1],
        param->connect.remote_bda[2],
        param->connect.remote_bda[3],
        param->connect.remote_bda[4],
        param->connect.remote_bda[5]);

    esp_ble_conn_update_params_t connectionParameters;
    bzero(&connectionParameters, sizeof(connectionParameters));
    memcpy(connectionParameters.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
    /* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
    connectionParameters.latency = 0;
    connectionParameters.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
    connectionParameters.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
    connectionParameters.timeout = 400;    // timeout = 400*10ms = 4000ms

    if (esp_ble_gap_update_conn_params(&connectionParameters) != ESP_OK)
    {
        ESP_LOGW(LOG_TAG, "Could not update connection parameters! Skipping.");
    }
}

void GattsApplication::handleGattsEventCreateAttributeTable(
    esp_gatt_if_t gatts_if,
    esp_ble_gatts_cb_param_t* param)
{
    if (param->add_attr_tab.status != ESP_GATT_OK)
    {
        throw std::runtime_error("error creating the GATT attribute table");
    }

    if (param->add_attr_tab.num_handle != m_nextServiceForRegistration->service->attributeTable().length)
    {
        throw std::runtime_error("unexpected number of handles registered");
    }

    m_nextServiceForRegistration->service->pushHandles(param->add_attr_tab.handles);
    if (esp_ble_gatts_start_service(param->add_attr_tab.handles[0]) != ESP_OK)
    {
        throw std::runtime_error("error starting the GATT service");
    }
}

void GattsApplication::handleGattsEventDisconnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGI(
        LOG_TAG,
        "DISCONNECT, conn_id=%d, address=%02x:%02x:%02x:%02x:%02x:%02x, reason=0x%04x",
        param->disconnect.conn_id,
        param->disconnect.remote_bda[0],
        param->disconnect.remote_bda[1],
        param->disconnect.remote_bda[2],
        param->disconnect.remote_bda[3],
        param->disconnect.remote_bda[4],
        param->disconnect.remote_bda[5],
        param->disconnect.reason);

    if (configurationDone())
    {
        esp_ble_gap_start_advertising(&advertisementParameters);
    }
    else
    {
        ESP_LOGW(LOG_TAG, "not starting advertising again. Configuration not yet completed");
    }
}

void GattsApplication::handleGattsEventMtu(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGD(LOG_TAG, "MTU, conn_id=%d, mtu=%d", param->mtu.conn_id, param->mtu.mtu);
}

void GattsApplication::handleGattsEventRead(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGD(LOG_TAG, "READ need_rsp=%d, handle=%04x", (int)param->read.need_rsp, param->read.handle);
    if (param->read.need_rsp)
    {
        esp_gatt_rsp_t response;
        bzero(&response, sizeof(response));
        response.attr_value.handle = param->read.handle;

        try
        {
            auto servicePointer = m_services;
            while (servicePointer)
            {
                if (servicePointer->service->hasHandle(param->read.handle))
                {
                    servicePointer->service->readCharacteristic(
                        param->read.handle,
                        response.attr_value.value,
                        &response.attr_value.len);

                    esp_ble_gatts_send_response(
                        gatts_if,
                        param->read.conn_id,
                        param->read.trans_id,
                        ESP_GATT_OK,
                        &response);
                    break;
                }

                servicePointer = servicePointer->next;
            }
            if (!servicePointer)
            {
                ESP_LOGW(LOG_TAG, "Could not find suitable service for handle %04x", param->read.handle);
                esp_ble_gatts_send_response(
                    gatts_if,
                    param->read.conn_id,
                    param->read.trans_id,
                    ESP_GATT_INVALID_HANDLE,
                    &response);
            }
        }
        catch(const std::exception& e)
        {
            ESP_LOGW(LOG_TAG, "Could not respond to read request: %s", e.what());
            esp_ble_gatts_send_response(
                gatts_if,
                param->read.conn_id,
                param->read.trans_id,
                ESP_GATT_INTERNAL_ERROR,
                &response);
        }

        ++m_dummyValue;
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

    m_nextServiceForRegistration = m_services;
    registerNextService(gatts_if);
}

void GattsApplication::handleGattsEventWrite(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGD(
        LOG_TAG,
        "WRITE is_prep=%d, need_rsp=%d, handle=%04x",
        param->write.is_prep,
        (int)param->write.need_rsp,
        param->write.handle);

    if (!param->write.is_prep)
    {

        try
        {
            auto servicePointer = m_services;
            while (servicePointer)
            {
                if (servicePointer->service->hasHandle(param->write.handle))
                {
                    servicePointer->service->writeCharacteristic(
                        param->write.handle,
                        param->write.value,
                        param->write.len);

                    if (param->write.need_rsp)
                    {
                        esp_ble_gatts_send_response(
                            gatts_if,
                            param->write.conn_id,
                            param->write.trans_id,
                            ESP_GATT_OK,
                            nullptr);
                    }
                    break;
                }

                servicePointer = servicePointer->next;
            }
            if (!servicePointer)
            {
                ESP_LOGW(LOG_TAG, "Could not find suitable service for handle %04x", param->write.handle);
                esp_ble_gatts_send_response(
                    gatts_if,
                    param->write.conn_id,
                    param->write.trans_id,
                    ESP_GATT_INVALID_HANDLE,
                    nullptr);
            }
        }
        catch(const std::exception& e)
        {
            ESP_LOGW(LOG_TAG, "Could not respond to write request: %s", e.what());
            esp_ble_gatts_send_response(
                gatts_if,
                param->write.conn_id,
                param->write.trans_id,
                ESP_GATT_INTERNAL_ERROR,
                nullptr);
        }
    }
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

    int uuid16ServiceCount = numberOfAdvertisedServices(BleUuid::Width::UUID_16);
    if (uuid16ServiceCount > 0)
    {
        requiredLength += 2 + 2 * uuid16ServiceCount;
    }
    int uuid32ServiceCount = numberOfAdvertisedServices(BleUuid::Width::UUID_32);
    if (uuid32ServiceCount > 0)
    {
        requiredLength += 2 + 4 * uuid32ServiceCount;
    }

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

    // put advertisement flags
    auto payloadPointer = m_rawAdvertisementData.payload;
    memcpy(payloadPointer, advertisementFlags, sizeof(advertisementFlags));
    payloadPointer += sizeof(advertisementFlags);

    // put short device name
    *payloadPointer = strlen(m_shortDeviceName) + 1;
    ++payloadPointer;
    *payloadPointer = 0x09;
    ++payloadPointer;
    memcpy(payloadPointer, m_shortDeviceName, strlen(m_shortDeviceName));
    payloadPointer += strlen(m_shortDeviceName);

    // put services
    if (uuid16ServiceCount > 0)
    {
        *payloadPointer = 1 + 2 * uuid16ServiceCount;
        ++payloadPointer;
        *payloadPointer = 0x03;
        ++payloadPointer;

        auto servicePointer = m_services;
        while (servicePointer)
        {
            auto& serviceId = servicePointer->service->serviceId();
            if (serviceId.width == BleUuid::Width::UUID_16 && serviceId.advertise)
            {
                memcpy(payloadPointer, &serviceId.uuid16, sizeof(serviceId.uuid16));
                payloadPointer += sizeof(serviceId.uuid16);
            }

            servicePointer = servicePointer->next;
        }
    }
    if (uuid32ServiceCount > 0)
    {
        *payloadPointer = 1 + 4 * uuid32ServiceCount;
        ++payloadPointer;
        *payloadPointer = 0x05;
        ++payloadPointer;

        auto servicePointer = m_services;
        while (servicePointer)
        {
            auto& serviceId = servicePointer->service->serviceId();
            if (serviceId.width == BleUuid::Width::UUID_32 && serviceId.advertise)
            {
                memcpy(payloadPointer, &serviceId.uuid32, sizeof(serviceId.uuid32));
                payloadPointer += sizeof(serviceId.uuid32);
            }

            servicePointer = servicePointer->next;
        }
    }
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
        throw std::runtime_error("error allocating memory for the scan response data");
    }
    m_rawScanResponseData.length = requiredLength;

    // put advertisement flags
    auto payloadPointer = m_rawScanResponseData.payload;
    memcpy(payloadPointer, advertisementFlags, sizeof(advertisementFlags));
    payloadPointer += sizeof(advertisementFlags);

    // put appearance
    *payloadPointer = 3;
    ++payloadPointer;
    *payloadPointer = 0x19;
    ++payloadPointer;
    memcpy(payloadPointer, &m_appearance, sizeof(m_appearance));
    payloadPointer += sizeof(m_appearance);
}

void GattsApplication::registerNextService(esp_gatt_if_t gatts_if)
{
    if (!m_nextServiceForRegistration)
    {
        ESP_LOGI(LOG_TAG, "Finished registering all services");
        return;
    }

    auto attributeTable = m_nextServiceForRegistration->service->attributeTable();
    if (esp_ble_gatts_create_attr_tab(
            attributeTable.table,
            gatts_if,
            attributeTable.length,
            m_nextServiceRegistrationNumber) != ESP_OK)
    {
        throw std::runtime_error("error registering the GATT attribute table");
    }
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
