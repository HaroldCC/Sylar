#include "../sylar/config/config.h"
#include "sylar/log/log.h"
#include "yaml-cpp/yaml.h"

sylar::ConfigVar<int>::ptr gIntValueConfig =
    sylar::Config::Lookup("system.port", static_cast<int>(8080), "system port");

sylar::ConfigVar<float>::ptr gFloatValueConfig =
    sylar::Config::Lookup("system.value", static_cast<float>(10.2f), "system value");

sylar::ConfigVar<std::vector<int>>::ptr gIntVecValueConfig =
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2, 3, 4, 5}, "system int vec");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsNull())
    {
        LOG_INFO(LOG_ROOT) << std::string(level * 4, ' ')
                           << " NULL - " << node.Type() << " - " << level;
    }
    else if (node.IsScalar())
    {
        LOG_INFO(LOG_ROOT) << std::string(level * 4, ' ')
                           << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            LOG_INFO(LOG_ROOT) << std::string(level * 4, ' ')
                               << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            LOG_INFO(LOG_ROOT) << std::string(level * 4, ' ')
                               << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml()
{
    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    print_yaml(root, 0);

    LOG_INFO(LOG_ROOT) << root.Scalar();
}

void test_config()
{
    LOG_INFO(LOG_ROOT) << "before: " << gIntValueConfig->getValue();
    LOG_INFO(LOG_ROOT) << "before: " << gFloatValueConfig->toString();

    auto v = gIntVecValueConfig->getValue();
    for (auto &i : v)
    {
        LOG_INFO(LOG_ROOT) << "before: "
                           << "int_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    LOG_INFO(LOG_ROOT) << "after: " << gIntValueConfig->getValue();
    LOG_INFO(LOG_ROOT) << "after: " << gFloatValueConfig->toString();
}
{

    //test_yaml();

    test_config();
    return 0;
}
