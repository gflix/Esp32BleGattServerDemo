#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>
#include <esp_log.h>
#include <stdexcept>
#include "BleServer.hpp"

#define LOG_TAG "BleServer"

#define BLE_GATT_LOCAL_MTU (400)

namespace Esp32
{

static BleServer bleServer;

static void gapEventCallbackWrapper(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
static void gattsEventCallbackWrapper(
    esp_gatts_cb_event_t event,
    esp_gatt_if_t gatts_if,
    esp_ble_gatts_cb_param_t* param);

BleServer::BleServer():
    m_gattsApplication(nullptr)
{
}

BleServer::~BleServer()
{
}

void BleServer::probe(void)
{
    ESP_LOGD(LOG_TAG, "BleServer::probe()");

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

    if (esp_ble_gatt_set_local_mtu(BLE_GATT_LOCAL_MTU) != ESP_OK)
    {
        throw std::runtime_error("error setting GATT MTU");
    }

    if (esp_ble_gap_register_callback(gapEventCallbackWrapper) != ESP_OK)
    {
        throw std::runtime_error("error registering GAP event handler");
    }

    if (esp_ble_gatts_register_callback(gattsEventCallbackWrapper) != ESP_OK)
    {
        throw std::runtime_error("error registering GATTS event handler");
    }
}

void BleServer::setGattsApplication(GattsApplication* gattsApplication)
{
    m_gattsApplication = gattsApplication;
    if (!m_gattsApplication)
    {
        throw std::runtime_error("no GATTS application registered");
    }

    if (esp_ble_gatts_app_register(m_gattsApplication->applicationId()) != ESP_OK)
    {
        throw std::runtime_error("error registering the GATTS application");
    }
}

void BleServer::gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
    if (!m_gattsApplication)
    {
        throw std::runtime_error("no GATTS application registered");
    }
    m_gattsApplication->gapEventCallback(event, param);
}

void BleServer::gattsEventCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    if (!m_gattsApplication)
    {
        throw std::runtime_error("no GATTS application registered");
    }
    m_gattsApplication->gattsEventCallback(event, gatts_if, param);
}

BleServer* BleServer::instance(void)
{
    return &bleServer;
}

void gapEventCallbackWrapper(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
    try
    {
        bleServer.gapEventCallback(event, param);
    }
    catch(const std::exception& e)
    {
        ESP_LOGE(LOG_TAG, "error handling GAP event: %s", e.what());
    }
}

void gattsEventCallbackWrapper(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param)
{
    try
    {
        bleServer.gattsEventCallback(event, gatts_if, param);
    }
    catch(const std::exception& e)
    {
        ESP_LOGE(LOG_TAG, "error handling GATTS event: %s", e.what());
    }
}

} /* namespace Esp32 */
