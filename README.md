# Esp32BleGattServerDemo

## Intention

This project is a rather trivial demo on how to implement a Bluetooth Low Energy GATT server on the ESP32 platform.
Any Bluetooth (stack) specific handling is hidden within a number of C++ classes which need to be instantiated and
configured on the users needs. This allows to reduce the user code to a very minimum. See the main application for an
example which implements a BLE GATT server application with 3 services and 4 characteristics (= attributes) each
representing a UInt16 value.

## Usage

### Usage of the demo

- Clone the repository
- Run 'idf.py set-target esp32'
- Then 'idf.py build' and
- Finally 'idf.py flash'

When scanning for Bluetooth devices a new device "ESP32" should be visible. The name gets updated to "ESP32-GATT-Demo"
on connect. There shall be 3 services with 4 characteristics in total. Some of them are writeable and keep their value
until the device is reset. The mobile application "nRF Connect" could be used for accessing the demo.

### Usage of the framework

The framework is written in C++ and makes use of exceptions to make the code more readable, reduce the number of
return values and give better information where errors are coming from.
To initialize a BLE GATT server application the following steps needs to be performed:

1. Initialize the non-volatile storage

```cpp
    NonVolatileStorage::instance()->probe();
```

2. Initialize the generic BLE server

```cpp
    BleServer::instance()->probe();
```

3. Initialize the characteristics
4. Initialize the services and register the characteristics within the services

```cpp
    gattsServiceA.addCharacteristic(&characteristicA1);
    gattsServiceA.addCharacteristic(&characteristicA2);
    gattsServiceB.addCharacteristic(&characteristicB);
    gattsServiceC.addCharacteristic(&characteristicC);
```

5. Initialize a GATT server application and register the services to the application

```cpp
    gattsApplication.addService(&gattsServiceA);
    gattsApplication.addService(&gattsServiceB);
    gattsApplication.addService(&gattsServiceC);
```

6. Register the GATT server application at the BLE server

```cpp
    BleServer::instance()->setGattsApplication(&gattsApplication);
```

After registering the GATT server application no characteristic and no service can be added to the application.
The Bluetooth stack automatically starts the internal registration process and finally starts to advertise the
GATT server application.

To actually get a useful application, one should implement classes which inherit from "GenericGattCharacteristic" and
override i.e. the read() and/or write() methods according to the users needs. The read() method gets called whenever a
specific characteristic shall return its value to the GATT client. The write() method gets called whenever a GATT
client tries to store a new value to the specific characteristic.
See UInt16GattCharacteristic for an example.

## Restrictions

The framework currently has the following (known) restrictions:

- Only 16 bit and 32 bit UUIDs are supported, there is currently no support for 128 bit UUIDs
- Notifications/Indications are currently not supported
- Applications cannot be modified after initialization, thus dynamic changes of services and their characteristics
  are not supported
- Manufacturer data is not yet supported
- The framework needs to allocate memory on the heap to store the registered services and characteristics in linked
  list; after initialization no dynamic memory handling is performed anymore

## Hints

### Restrict the number of advertised services and keep advertised device names short

The advertisement and scan response data sets are restricted to 31 bytes each. Since the service advertisements and
the short device name are placed within the advertisement data set the framework might throw an exception due to
exceeding the limit. Pleace reduce the number of bytes within the advertisement data set by:

- Shortening the device name to a reasonable minimum and/or
- Only advertise services which are required for a reasonable filtering on the client side

### Use 16 bit UUIDs only when implementing well known services and characteristics

The Bluetooth Special Interest Group assigned a number of 16 bit UUIDs to a number of well known services and 
characteristics (i.e. the battery level service), see https://www.bluetooth.com/de/specifications/assigned-numbers/ .
Whenever possible avoid to use custom 16 bit values when implementing custom services and characteristics to avoid
unambiguities. Use 32 bit UUIDs for custom services and characteristics instead.

### Avoid to define GATT server application, services and characteristics on the stack

A user may tend to define the GATT server application, the services and/or the characteristics within the app_main()
method or any other method or task. Please be aware, that defining any variable within a method will define it on
the stack of the current method. If the method is left, the class instances are destructed and cannot be used anymore.
Instead place the instances on the heap using "new" or, as shown in the demo, place the instances using "static"
keword within the source file where needed.
