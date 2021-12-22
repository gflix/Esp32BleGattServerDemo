#include <stdio.h>
#include <esp_log.h>
#include <stdexcept>
#include "BleServer.hpp"
#include "NonVolatileStorage.hpp"

#define LOG_TAG "Main"

extern "C" {

void app_main(void)
{
    ESP_LOGI(LOG_TAG, "ESP32/C++ - BLE test");
    ESP_LOGI(LOG_TAG, "====================");

    try
    {
        Esp32::NonVolatileStorage::instance()->probe();
        ESP_LOGI(LOG_TAG, "NonVolatileStorage: probing done");
        Esp32::BleServer::instance()->probe();
        ESP_LOGI(LOG_TAG, "BleServer: probing done");
    }
    catch(const std::exception& e)
    {
        ESP_LOGE(LOG_TAG, "Caught exception: %s\n", e.what());
    }    
}

}
