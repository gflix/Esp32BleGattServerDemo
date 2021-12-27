#ifndef MAIN_BLEUUID_HPP_
#define MAIN_BLEUUID_HPP_

#include <stdint.h>

namespace Esp32
{

struct BleUuid
{
    enum class Width
    {
        UUID_16,
        UUID_32,
    };

    BleUuid(
        Width width,
        uint32_t uuid);
    virtual ~BleUuid();

    Width width;
    uint16_t uuid16;
    uint32_t uuid32;
};

} /* namespace Esp32 */

#endif /* MAIN_BLEUUID_HPP_ */
