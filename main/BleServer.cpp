#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_log.h>
#include <stdexcept>
#include "BleServer.hpp"

#define LOG_TAG "BleServer"

namespace Esp32
{

static BleServer bleServer;

BleServer::BleServer()
{
}

BleServer::~BleServer()
{
}

void BleServer::probe(void)
{
    ESP_LOGI(LOG_TAG, "BleServer::probe()");

    if (esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT) != ESP_OK)
    {
        throw std::runtime_error("error releasing non-necessary memory from the bluetooth controller");
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK)
    {
        throw std::runtime_error("error initializing the bluetooth controller");
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_BLE) != ESP_OK)
    {
        throw std::runtime_error("error switching the bluetooth controller to BLE mode");
    }

    if (esp_bluedroid_init() != ESP_OK)
    {
        throw std::runtime_error("error initializing the bluetooth stack");
    }

    if (esp_bluedroid_enable() != ESP_OK)
    {
        throw std::runtime_error("error enabling the bluetooth stack");
    }
}

BleServer* BleServer::instance(void)
{
    return &bleServer;
}

} /* namespace Esp32 */
