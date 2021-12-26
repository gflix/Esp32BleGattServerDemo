#include <stdio.h>
#include <esp_log.h>
#include <stdexcept>
#include "BleServer.hpp"
#include "GattsApplication.hpp"
#include "GattsService.hpp"
#include "NonVolatileStorage.hpp"
#include "UInt16GattCharacteristic.hpp"

#define LOG_TAG "Main"

#define GATTS_APPLICATION_ID (0x2104)
#define GATTS_DEVICE_APPEARANCE (0x0280)

using namespace Esp32;

static GattsApplication gattsApplication(
    GATTS_APPLICATION_ID,
    "ESP32",
    "ESP32-GATT-Demo",
    GATTS_DEVICE_APPEARANCE);

static GattsService gattsServiceA(BleServiceUuid(BleUuid::Width::UUID_32, 0x21040001));
static GattsService gattsServiceB(BleServiceUuid(BleUuid::Width::UUID_32, 0x21040002, false));
static GattsService gattsServiceC(BleServiceUuid(BleUuid::Width::UUID_32, 0x21040003));

static UInt16GattCharacteristic characteristicA1(
    BleUuid(BleUuid::Width::UUID_32, 0x21041000),
    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
    "Foo",
    0x4142);
static UInt16GattCharacteristic characteristicA2(
    BleUuid(BleUuid::Width::UUID_16, 0x4020),
    ESP_GATT_PERM_READ,
    "Bar",
    0x3132);
static UInt16GattCharacteristic characteristicB(
    BleUuid(BleUuid::Width::UUID_16, 0x4110),
    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
    "Baz",
    0x6162);
static UInt16GattCharacteristic characteristicC(
    BleUuid(BleUuid::Width::UUID_32, 0x21043000),
    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
    nullptr,
    0x4f58);

extern "C" {

void app_main(void)
{
    ESP_LOGI(LOG_TAG, "ESP32/C++ - BLE test");
    ESP_LOGI(LOG_TAG, "====================");

    try
    {
        NonVolatileStorage::instance()->probe();
        ESP_LOGI(LOG_TAG, "NonVolatileStorage: probing done");
        BleServer::instance()->probe();
        ESP_LOGI(LOG_TAG, "BleServer: probing done");

        gattsServiceA.addCharacteristic(&characteristicA1);
        gattsServiceA.addCharacteristic(&characteristicA2);
        gattsApplication.addService(&gattsServiceA);

        gattsServiceB.addCharacteristic(&characteristicB);
        gattsApplication.addService(&gattsServiceB);

        gattsServiceC.addCharacteristic(&characteristicC);
        gattsApplication.addService(&gattsServiceC);

        BleServer::instance()->setGattsApplication(&gattsApplication);
        ESP_LOGI(LOG_TAG, "BleServer: GATTS application successfully set");
    }
    catch(const std::exception& e)
    {
        ESP_LOGE(LOG_TAG, "Caught exception: %s\n", e.what());
    }
}

}
