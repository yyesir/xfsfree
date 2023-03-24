#ifndef _INISETTING_H__
#define _INISETTING_H__

#include <string>

class inisetting
{
private:
    inisetting(){}
public:
    static inisetting* getInstance()
    {
        static inisetting _is;
        return &_is;
    }
    virtual ~inisetting(){}

    std::string getSPIPath(const char* logical_services);
};



#endif
