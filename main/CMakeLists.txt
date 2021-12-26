idf_component_register(
    SRCS
    Esp32BleGattServerDemo.cpp
    BleServer.cpp
    BleServiceUuid.cpp
    BleUuid.cpp
    GattsApplication.cpp
    GattsService.cpp
    GenericGattCharacteristic.cpp
    NonVolatileStorage.cpp
    UInt16GattCharacteristic.cpp
    INCLUDE_DIRS "."
)
