#include "GattsService.hpp"

namespace Esp32
{

GattsService::GattsService(uint16_t serviceId):
    m_serviceId(serviceId)
{
}

GattsService::~GattsService()
{
}

uint16_t GattsService::serviceId(void) const
{
    return m_serviceId;
}

} /* namespace Esp32 */
