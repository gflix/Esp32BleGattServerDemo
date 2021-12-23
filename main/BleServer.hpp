#ifndef MAIN_BLESERVER_HPP_
#define MAIN_BLESERVER_HPP_

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include "GattsApplication.hpp"

namespace Esp32
{

class BleServer
{
public:
    BleServer();
    virtual ~BleServer();

    void probe(void);
    void setGattsApplication(GattsApplication* gattsApplication);

    void gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
    void gattsEventCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);

    static BleServer* instance(void);

protected:

    GattsApplication* m_gattsApplication;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_BLESERVER_HPP_ */
