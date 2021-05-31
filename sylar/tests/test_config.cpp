#include "../sylar/config/config.h"
#include "sylar/log/log.h"

sylar::ConfigVar<int>::ptr gIntValueConfig =
    sylar::Config::Lookup("sylar.port", static_cast<int>(8080), "system port");

sylar::ConfigVar<int>::ptr gIntFloatConfig =
    sylar::Config::Lookup("system.port", static_cast<int>(8080), "system port");

int main(int argc, char const *argv[])
{
    LOG_INFO(LOG_ROOT) << gIntValueConfig->getValue();
    LOG_INFO(LOG_ROOT) << gIntFloatConfig->toString();

    return 0;
}
