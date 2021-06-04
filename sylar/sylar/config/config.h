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

    template <typename FromType, typename ToType>
    class LexicalCast
    {
    public:
        ToType operator()(const FromType &val)
        {
            return Util::lexical_cast<ToType>(val);
        }
    };

    template <typename T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &val)
        {
            YAML::Node node = YAML::Load(val);
            std::stringstream ss;
            std::vector<T> vec;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

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
    //

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