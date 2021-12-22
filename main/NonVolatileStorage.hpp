#ifndef MAIN_NONVOLATILESTORAGE_HPP_
#define MAIN_NONVOLATILESTORAGE_HPP_

namespace Esp32
{

class NonVolatileStorage
{
public:
    NonVolatileStorage();
    virtual ~NonVolatileStorage();

    void probe(void);

    static NonVolatileStorage* instance(void);

protected:

private:

};

} /* namespace Esp32 */

#endif /* MAIN_NONVOLATILESTORAGE_HPP_ */
