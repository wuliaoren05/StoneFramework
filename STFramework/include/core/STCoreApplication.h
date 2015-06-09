#ifndef ST_CORE_APPLICATION_H
#define ST_CORE_APPLICATION_H

#include "STObject.h"
#include "core/STEventDispatcher.h"

class STCoreApplication : public STObject, STEventDispatcher
{

public:
    STCoreApplication(int &argc, char **argv);
    ~STCoreApplication();

    STString applicationName() const;

    void exec();
    void exit();

private:
    class CoreApplicationPrivateData;
    STScopePtr<CoreApplicationPrivateData> m_data;
};

#endif // ST_CORE_APPLICATION_H
