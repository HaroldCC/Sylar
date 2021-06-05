#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include "../util/util.h"
#include "../log/log.h"
#include "yaml-cpp/yaml.h"
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace sylar
{
    class ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;

        ConfigVarBase(const std::string &name, const std::string &descroption = "")
            : m_name(name), m_description(descroption)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }

        virtual ~ConfigVarBase() {}

        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string &str) = 0;
        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    template <typename FromType, typename ToType>
    class LexicalCast
    {
    public:
        ToType operator()(const FromType &val)
        {
            return Util::lexical_cast<ToType>(val);
        }
    };

    // string to vector
    template <typename T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::vector<T> res;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                res.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return res;
        }
    };

    // vector to string
    template <typename T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &val)
        {
            YAML::Node node;
            for (auto &i : val)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to list
    template <typename T>
    class LexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::list<T> res;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                res.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return res;
        }
    };

    // list to string
    template <typename T>
    class LexicalCast<std::list<T>, std::string>
    {
    public:
        std::string operator()(const std::list<T> &val)
        {
            YAML::Node node;
            for (auto &i : val)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to set
    template <typename T>
    class LexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::set<T> res;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                res.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return res;
        }
    };

    // set to string
    template <typename T>
    class LexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T> &val)
        {
            YAML::Node node;
            for (auto &i : val)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to unordered_set
    template <typename T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::unordered_set<T> res;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                res.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return res;
        }
    };

    // unordered_set to string
    template <typename T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T> &val)
        {
            YAML::Node node;
            for (auto &i : val)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to map
    template <typename T>
    class LexicalCast<std::string, std::map<std::string, T>>
    {
    public:
        std::map<std::string, T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::map<std::string, T> res;
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                ss.str("");
                ss << it->second;
                res.insert(std::make_pair(it->first.Scalar(),
                                          LexicalCast<std::string, T>()(ss.str())));
            }
            return res;
        }
    };

    // map to string
    template <typename T>
    class LexicalCast<std::map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::map<std::string, T> &val)
        {
            YAML::Node node;
            for (auto &i : val)
            {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // string to unordered_map
    template <typename T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::unordered_map<std::string, T> res;
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                ss.str("");
                ss << it->second;
                res.insert(std::make_pair(it->first.Scalar(),
                                          LexicalCast<std::string, T>()(ss.str())));
            }
            return res;
        }
    };

    // unordered_map to string
    template <typename T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &val)
        {
            YAML::Node node;
            for (auto &i : val)
            {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // FromStr: T operator()(cosnt std::string&)
    // ToStr: std::string operator()(const T&)
    template <typename T, typename FormStr = LexicalCast<std::string, T>,
              typename ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVar>;

        ConfigVar(const std::string &name, const T &defaultValue, const std::string &descripton = "")
            : ConfigVarBase(name, descripton), m_val(defaultValue) {}

        std::string toString() override
        {
            try
            {
                //return Util::lexical_cast<std::string>(m_val);
                return ToStr()(m_val);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR(LOG_ROOT) << "ConfigVar::toString exception " << e.what()
                                    << " convert: " << typeid(m_val).name() << "to string"
                                    << '\n';
            }

            return "";
        }

        bool fromString(const std::string &str) override
        {
            try
            {
                //return (m_val = Util::lexical_cast<T>(str));
                setValue(FormStr()(str));
            }
            catch (const std::exception &e)
            {
                LOG_ERROR(LOG_ROOT) << "ConfigVar::fromString exception " << e.what()
                                    << " convert: string to " << typeid(m_val).name()
                                    << '\n';
            }
            return false;
        }

        const T getValue() const { return m_val; }
        void setValue(const T &val) { m_val = val; }
        std::string getTypeName() const override { return typeid(T).name(); }

    private:
        T m_val;
    };

    class Config
    {
    public:
        using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;

        template <typename T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name,
                                                 const T &defaultValue,
                                                 const std::string &description = "")
        {
            auto it = m_datas.find(name);
            if (it != m_datas.end())
            {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                if (tmp)
                {
                    LOG_INFO(LOG_ROOT) << "Config::Lookup name = " << name << " exists\n";
                    return tmp;
                }
                else
                {
                    LOG_ERROR(LOG_ROOT) << "Config::Lookup name = " << name << " exists but type not "
                                        << typeid(T).name() << ", real type = "
                                        << it->second->getTypeName() << " value: " << it->second->toString();
                    return nullptr;
                }
            }

            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos)
            {
                LOG_ERROR(LOG_ROOT) << "Config::Lookup name invaild " << name << " \n";
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, defaultValue, description));
            m_datas[name] = v;

            return v;
        }

        template <typename T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name)
        {
            auto it = m_datas.find(name);
            if (it == m_datas.end())
            {
                return nullptr;
            }

            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        static void LoadFromYaml(const YAML::Node &root);

        static ConfigVarBase::ptr LookupBase(const std::string &name);

    private:
        static ConfigVarMap m_datas;
    };

}

#endif // __CONFIG_H__