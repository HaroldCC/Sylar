#include "../sylar/config/config.h"
#include "sylar/log/log.h"
#include "yaml-cpp/yaml.h"

sylar::ConfigVar<int>::ptr gIntValueConfig =
    sylar::Config::Lookup("system.port", static_cast<int>(8080), "system port");

sylar::ConfigVar<float>::ptr gIntValueConfigD =
    sylar::Config::Lookup("system.port", static_cast<float>(8080), "system port");

sylar::ConfigVar<float>::ptr gFloatValueConfig =
    sylar::Config::Lookup("system.value", static_cast<float>(10.2f), "system value");

sylar::ConfigVar<std::vector<int>>::ptr gIntVecValueConfig =
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2, 3, 4, 5}, "system int vec");

sylar::ConfigVar<std::list<int>>::ptr gIntListValueConfig =
    sylar::Config::Lookup("system.int_list", std::list<int>{11, 22, 33, 44}, "system int list");

sylar::ConfigVar<std::set<int>>::ptr gIntSetValueConfig =
    sylar::Config::Lookup("system.int_set", std::set<int>{11, 22, 33, 44}, "system int set ");

sylar::ConfigVar<std::unordered_set<int>>::ptr gIntUSetValueConfig =
    sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{11, 22, 33, 44}, "system int uset ");

sylar::ConfigVar<std::map<std::string, int>>::ptr gStrIntMapValueConfig =
    sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"key", 12}, {"key", 12}},
                          "system str int map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr gStrIntUMapValueConfig =
    sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"key", 12}, {"key", 12}},
                          "system str int umap");

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
    YAML::Node root = YAML::LoadFile("../bin/conf/testConfig.yml");
    print_yaml(root, 0);

    LOG_INFO(LOG_ROOT) << root.Scalar();
}

void test_config()
{
    LOG_INFO(LOG_ROOT) << "before: " << gIntValueConfig->getValue();
    LOG_INFO(LOG_ROOT) << "before: " << gFloatValueConfig->toString();

    // 由于会多次使用宏,会对v进行重定义，所以加上大括号进行包装

#define XX(g_val, name, prefix)                                \
    {                                                          \
        auto &v = g_val->getValue();                           \
        for (auto &i : v)                                      \
        {                                                      \
            LOG_INFO(LOG_ROOT) << #prefix " " #name ": " << i; \
        }                                                      \
        LOG_INFO(LOG_ROOT) << #prefix " " #name " yaml: "      \
                           << g_val->toString();               \
    }

#define XX_M(g_val, name, prefix)                                      \
    {                                                                  \
        auto &v = g_val->getValue();                                   \
        for (auto &i : v)                                              \
        {                                                              \
            LOG_INFO(LOG_ROOT) << #prefix " " #name ": {"              \
                               << i.first << " - " << i.second << "}"; \
        }                                                              \
        LOG_INFO(LOG_ROOT) << #prefix " " #name " yaml: "              \
                           << g_val->toString();                       \
    }

    XX(gIntVecValueConfig, int_vec, before);
    XX(gIntListValueConfig, int_list, before);
    XX(gIntSetValueConfig, int_set, before);
    XX(gIntUSetValueConfig, int_uset, before);
    XX_M(gStrIntMapValueConfig, str_int_map, before);
    XX_M(gStrIntUMapValueConfig, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("../bin/conf/testConfig.yml");
    sylar::Config::LoadFromYaml(root);

    LOG_INFO(LOG_ROOT) << "after: " << gIntValueConfig->getValue();
    LOG_INFO(LOG_ROOT) << "after: " << gFloatValueConfig->toString();

    XX(gIntVecValueConfig, int_vec, after);
    XX(gIntListValueConfig, int_list, after);
    XX(gIntSetValueConfig, int_set, after);
    XX(gIntUSetValueConfig, int_uset, after);
    XX_M(gStrIntMapValueConfig, str_int_map, after);
    XX_M(gStrIntUMapValueConfig, str_int_umap, after);

#undef XX
#undef XX_M
}

class Person
{
public:
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const
    {
        std::stringstream ss;
        ss << "[Person name = " << m_name
           << " age = " << m_age
           << " sex = " << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person &rhs) const
    {
        return m_name == rhs.m_name &&
               m_age == rhs.m_age &&
               m_sex == rhs.m_sex;
    }
};

// 对自定义类型进行LexicalCast偏特化
namespace sylar
{
    template <>
    class LexicalCast<std::string, Person>
    {
    public:
        Person operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            Person p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
            return p;
        }
    };

    template <>
    class LexicalCast<Person, std::string>
    {
    public:
        std::string operator()(const Person &p)
        {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
}

// 如果想要使用自定义类型的配置项，需要对自定义类型进行LexicalCast偏特化
sylar::ConfigVar<Person>::ptr g_person =
    sylar::Config::Lookup("class.person", Person(), "class person");

// 自定义类型和stl容器嵌套测试
sylar::ConfigVar<std::map<std::string, Person>>::ptr g_person_map =
    sylar::Config::Lookup("class.map", std::map<std::string, Person>(), "class person");

sylar::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map =
    sylar::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "class person");

void test_class()
{
    LOG_INFO(LOG_ROOT) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_PM(g_var, prefix)                                                                 \
    {                                                                                        \
        auto m = g_person_map->getValue();                                                   \
        for (auto &i : m)                                                                    \
        {                                                                                    \
            LOG_INFO(LOG_ROOT) << prefix << ": " << i.first << " - " << i.second.toString(); \
        }                                                                                    \
        LOG_INFO(LOG_ROOT) << prefix << ": size = " << m.size();                             \
    }

    g_person->addListener(10, [](const Person &oldValue, const Person &newValue)
                          { LOG_INFO(LOG_ROOT) << "old_value: " << oldValue.toString()
                                               << " new_value: " << newValue.toString(); });

    XX_PM(g_person_map, "class.map before");
    LOG_INFO(LOG_ROOT) << "before: " << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("../bin/conf/testConfig.yml");
    sylar::Config::LoadFromYaml(root);

    LOG_INFO(LOG_ROOT) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    LOG_INFO(LOG_ROOT) << "after: " << g_person_vec_map->toString();
}

void test_log_yaml_config()
{
    static sylar::Logger::ptr system_log = LOG_NAME("system");
    LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("../bin/conf/log.yaml");
    sylar::Config::LoadFromYaml(root);
    std::cout << "======================================" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << root << std::endl;
    LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");
    LOG_INFO(system_log) << "hello system" << std::endl;
}

int main(int argc, char const *argv[])
{

    //test_yaml();
    //test_config();
    //test_class();
    test_log_yaml_config();

    return 0;
}
