#include <esp_log.h>
#include <nvs_flash.h>
#include <stdexcept>
#include "NonVolatileStorage.hpp"

#define LOG_TAG "NonVolatileStorage"

namespace Esp32
{

static NonVolatileStorage nonVolatileStorage;

NonVolatileStorage::NonVolatileStorage()
{
}

NonVolatileStorage::~NonVolatileStorage()
{
}

void NonVolatileStorage::probe(void)
{
    ESP_LOGD(LOG_TAG, "NonVolatileStorage::probe()");

    auto returnCode = nvs_flash_init();
    if (returnCode == ESP_ERR_NVS_NO_FREE_PAGES ||
        returnCode == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        if (nvs_flash_erase() != ESP_OK)
        {
            throw std::runtime_error("error erasing non-volatile storage before re-initialization");
        }
        returnCode = nvs_flash_init();
    }
    if (returnCode != ESP_OK)
    {
        throw std::runtime_error("error initializing non-volatile storage");
    }
}

NonVolatileStorage* NonVolatileStorage::instance(void)
{
    return &nonVolatileStorage;
}

} /* namespace Esp32 */
