#ifndef MAIN_GATTSAPPLICATION_HPP_
#define MAIN_GATTSAPPLICATION_HPP_

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include "GattsService.hpp"

#define GATTS_APPLICATION_DEFAULT_APPEARANCE (0x0000)

namespace Esp32
{

class GattsApplication
{
public:
    struct AdvertisementData
    {
        AdvertisementData(uint8_t* payload = nullptr, size_t length = 0);

        uint8_t* payload;
        size_t length;

        void dump(void) const;
    };

    struct ServiceList
    {
        ServiceList(GattsService* service);

        GattsService* service;
        ServiceList* next;
    };

    GattsApplication(
        uint16_t applicationId,
        const char* shortDeviceName,
        const char* fullDeviceName = nullptr,
        uint16_t appearance = GATTS_APPLICATION_DEFAULT_APPEARANCE);
    virtual ~GattsApplication();

    uint16_t applicationId(void) const;

    void addService(GattsService* service);
    int numberOfAdvertisedServices(BleUuid::Width width) const;

    void gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
    void gattsEventCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);

    const static uint8_t advertisementFlags[3];
    static esp_ble_adv_params_t advertisementParameters;

protected:

    uint16_t m_applicationId;
    const char* m_shortDeviceName;
    const char* m_fullDeviceName;
    uint16_t m_appearance;

    ServiceList* m_services;
    ServiceList* m_nextServiceForRegistration;
    uint8_t m_nextServiceRegistrationNumber;

    uint8_t m_configurationDone;
    esp_gatt_if_t m_interface;
    AdvertisementData m_rawAdvertisementData;
    AdvertisementData m_rawScanResponseData;

    uint8_t m_dummyValue;

    void handleGapEventAdvertisementDataSetComplete(void);
    void handleGapEventScanResponseDataSetComplete(void);
    void handleGapEventAdvertisementStartComplete(esp_ble_gap_cb_param_t* param);
    void handleGapEventUpdatedConnectionParameters(esp_ble_gap_cb_param_t* param);

    void handleGattsEventConnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventCreateAttributeTable(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventDisconnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventMtu(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventRead(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventRegister(esp_gatt_if_t gatts_if);
    void handleGattsEventWrite(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);

    void generateRawAdvertisementData(void);
    void generateRawScanResponseData(void);

    void registerNextService(esp_gatt_if_t gatts_if);

    void setConfigurationAdvertisementPendingFlag(void);
    void setConfigurationAdvertisementDoneFlag(void);
    void setConfigurationScanResponsePendingFlag(void);
    void setConfigurationScanResponseDoneFlag(void);
    bool configurationDone(void) const;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GATTSAPPLICATION_HPP_ */
