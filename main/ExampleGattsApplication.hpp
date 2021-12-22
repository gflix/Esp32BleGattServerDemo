#ifndef MAIN_EXAMPLEGATTSAPPLICATION_HPP_
#define MAIN_EXAMPLEGATTSAPPLICATION_HPP_

#include "GenericGattsApplication.hpp"

namespace Esp32
{

class ExampleGattsApplication: public GenericGattsApplication
{
public:
    ExampleGattsApplication();
    virtual ~ExampleGattsApplication();

    static const uint16_t GATTS_SERVICE_UUID_TEST_A;
    static const uint16_t GATTS_CHAR_UUID_TEST_A;
    static const uint16_t GATTS_CHAR_UUID_TEST_B;

protected:

    void registerAttibuteTable(esp_gatt_if_t gatts_if) override;
    void startService(esp_ble_gatts_cb_param_t* param) override;
    void writeAttribute(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) override;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_EXAMPLEGATTSAPPLICATION_HPP_ */
