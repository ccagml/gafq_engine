#ifndef __GAMECONFIG_H__
#define __GAMECONFIG_H__
#include <boost/property_tree/ptree.hpp>
#include <string>

class GameConfig
{
private:
public:
    // 构造函数
    GameConfig()
    {
        objectCount++;
        std::cout << "create" << std::endl;
    }
    ~GameConfig()
    {
        objectCount--;
        std::cout << "delete" << std::endl;
    }
    static GameConfig *cfg;
    static int objectCount;
    static GameConfig *get_instance()
    {
        if (cfg == nullptr)
        {
            std::cout << "new" << std::endl;
            cfg = new GameConfig();
        }
        return cfg;
    }
    int get_config_int(std::string key);
    std::string get_config_string(std::string key);
    float get_config_float(std::string key);
    void load_config(std::string path);
    boost::property_tree::ptree pt;
    boost::property_tree::ptree get_config_pt();
};

#endif