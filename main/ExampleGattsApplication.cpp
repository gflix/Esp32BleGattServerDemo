#include "ExampleGattsApplication.hpp"

#define BLE_EXAMPLE_APPLICATION_ID (0x55)

namespace Esp32
{

ExampleGattsApplication::ExampleGattsApplication():
    GenericGattsApplication(BLE_EXAMPLE_APPLICATION_ID)
{
}

ExampleGattsApplication::~ExampleGattsApplication()
{
}

} /* namespace Esp32 */
