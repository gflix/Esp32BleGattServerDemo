#ifndef MAIN_GATTSSERVICE_HPP_
#define MAIN_GATTSSERVICE_HPP_

#include <esp_gatts_api.h>

namespace Esp32
{

class GattsService
{
public:
    GattsService(uint16_t serviceId);
    virtual ~GattsService();

    uint16_t serviceId(void) const;

protected:

    uint16_t m_serviceId;

private:

};

} /* namespace Esp32 */

#endif /* MAIN_GATTSSERVICE_HPP_ */
