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

protected:

    uint16_t m_applicationId;
    uint8_t m_configurationDone;
    esp_gatt_if_t m_interface;

    void handleGapEventRawAdvertisingDataSetComplete(void);
    void handleGapEventRawScanResponseDataSetComplete(void);
    void handleGapEventAdvertisementStartComplete(esp_ble_gap_cb_param_t* param);
    void handleGapEventUpdatedConnectionParameters(esp_ble_gap_cb_param_t* param);

    void handleGattsEventConnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventDisconnect(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventMtu(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void handleGattsEventRegister();

    void setConfigurationAdvertisingPendingFlag(void);
    void setConfigurationAdvertisingDoneFlag(void);
    void setConfigurationScanResponsePendingFlag(void);
    void setConfigurationScanResponseDoneFlag(void);
    bool configurationDone(void) const;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GENERICGATTSAPPLICATION_HPP_ */
