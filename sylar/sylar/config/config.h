#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include "../util/util.h"
#include "../log/log.h"
#include "yaml-cpp/yaml.h"

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

    protected:
        std::string m_name;
        std::string m_description;
    };

    template <typename T>
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
                return Util::lexical_cast<std::string>(m_val);
            }
            catch (const std::exception &e)
            {
                LOG_ERROR(LOG_ROOT) << "ConfigVar::toString exception " << e.what()
                                    << " convert: " << typeid(m_val).name() << "to string"
                                    << '\n';
            }

            return "";
        }

        const T getValue() const { return m_val; }
        void setValue(const T &val) { m_val = val; }

        bool fromString(const std::string &str) override
        {
            try
            {
                return (m_val = Util::lexical_cast<T>(str));
            }
            catch (const std::exception &e)
            {
                LOG_ERROR(LOG_ROOT) << "ConfigVar::fromString exception " << e.what()
                                    << " convert: string to " << typeid(m_val).name()
                                    << '\n';
            }
            return false;
        }

    private:
        T m_val;
    };

    class Config
    {
    public:
        using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

        template <typename T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name,
                                                 const T &defaultValue,
                                                 const std::string &description = "")
        {
            auto tmp = Lookup<T>(name);
            if (tmp)
            {
                LOG_INFO(LOG_ROOT) << "Config::Lookup name = " << name << " exists\n";
                return tmp;
            }

            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos)
            {
                LOG_ERROR(LOG_ROOT) << "Lookup name invaild " << name << " \n";
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