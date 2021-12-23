#ifndef MAIN_GENERICGATTSAPPLICATION_HPP_
#define MAIN_GENERICGATTSAPPLICATION_HPP_

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>

namespace Esp32
{

class GenericGattsApplication
{
public:
    GenericGattsApplication(uint16_t applicationId);
    virtual ~GenericGattsApplication();

    uint16_t applicationId(void) const;
    esp_gatt_if_t interface(void) const;

    void gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
    void gattsEventCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);

    static const uint16_t primary_service_uuid;
    static const uint16_t character_declaration_uuid;
    static const uint16_t character_client_config_uuid;
    static const uint16_t character_description_uuid;
    static const uint8_t char_prop_read;
    static const uint8_t char_prop_write;
    static const uint8_t char_prop_read_write_notify;

protected:

    uint16_t m_applicationId;
    uint8_t m_configurationDone;
    esp_gatt_if_t m_interface;

    void handleGapEventRawAdvertisingDataSetComplete(void);
    void handleGapEventRawScanResponseDataSetComplete(void);
    void handleGapEventAdvertisementStartComplete(esp_ble_gap_cb_param_t* param);
    void handleGapEventUpdatedConnectionParameters(esp_ble_gap_cb_param_t* param);

    void handleGattsEventConnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventCreateAttributeTable(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventDisconnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventMtu(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventRegister(esp_gatt_if_t gatts_if);
    void handleGattsEventWrite(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);

    virtual void registerAttibuteTable(esp_gatt_if_t gatts_if) = 0;
    virtual void startService(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) = 0;
    virtual void writeAttribute(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) = 0;

    void setConfigurationAdvertisingPendingFlag(void);
    void setConfigurationAdvertisingDoneFlag(void);
    void setConfigurationScanResponsePendingFlag(void);
    void setConfigurationScanResponseDoneFlag(void);
    bool configurationDone(void) const;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GENERICGATTSAPPLICATION_HPP_ */
