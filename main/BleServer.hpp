#ifndef MAIN_BLESERVER_HPP_
#define MAIN_BLESERVER_HPP_

namespace Esp32
{

class BleServer
{
public:
    BleServer();
    virtual ~BleServer();

    void probe(void);

    static BleServer* instance(void);

protected:

private:

};

} /* namespace Esp32 */

#endif /* MAIN_BLESERVER_HPP_ */
