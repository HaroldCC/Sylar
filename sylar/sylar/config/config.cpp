#include "../config/config.h"

namespace sylar
{

    static void ListAllMember(const std::string &prefix, const YAML::Node &node,
                              std::list<std::pair<std::string, const YAML::Node>> &output)
    {
        if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") !=
            std::string ::npos)
        {
            LOG_ERROR(LOG_ROOT) << "Config invalid name: " << prefix << " : " << node;
            return;
        }

        output.push_back(std::make_pair(prefix, node));
        if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                ListAllMember(prefix.empty() ? it->first.Scalar()
                                             : prefix + "." + it->first.Scalar(),
                              it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node &root)
    {
        std::list<std::pair<std::string, const YAML::Node>> allNodes;
        ListAllMember("", root, allNodes);

        for (auto &i : allNodes)
        {
            std::string key = i.first;
            if (key.empty())
            {
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);
            if (var)
            {
                if (i.second.IsScalar())
                {
                    var->fromString(i.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    ConfigVarBase::ptr Config::LookupBase(const std::string &name)
    {
        auto it = getDatas().find(name);
        return it == getDatas().end() ? nullptr : it->second;
    }
}