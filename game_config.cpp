#include "game_config.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>

int GameConfig::objectCount = 0;
GameConfig *GameConfig::cfg = nullptr;

int GameConfig::get_config_int(std::string key) { return this->pt.get<int>(key); }

float GameConfig::get_config_float(std::string key) { return this->pt.get<float>(key); }

std::string GameConfig::get_config_string(std::string key) { return this->pt.get<std::string>(key); }

boost::property_tree::ptree GameConfig::get_config_pt() { return this->pt; }

void GameConfig::load_config(std::string path)
{
    std::ifstream jsonFile(path);
    boost::property_tree::ptree pt;
    read_json(jsonFile, pt);
    this->pt = pt;
    // std::stringstream ss;
    // boost::property_tree::json_parser::write_json(ss, pt);
    // std::cout << ss.str() << std::endl;
}

// int main(int argc, char const *argv[])
// {
//     /* code */

//     GameConfig::get_instance()->load_config("./config.json");
//     std::cout << "0:name:" << GameConfig::get_instance()->get_config_string("name") << std::endl;
//     std::cout << "0:object.currency:" << GameConfig::get_instance()->get_config_string("object.currency") << std::endl;
//     std::cout << "1:pi:" << GameConfig::get_instance()->get_config_float("pi") << std::endl;

//     boost::property_tree::ptree config_pt = GameConfig::get_instance()->get_config_pt();

//     std::stringstream ss;
//     boost::property_tree::json_parser::write_json(ss, config_pt);
//     std::cout << ss.str() << std::endl;
//     // std::cout << "2:pt:" << config_pt.get() << std::endl;

//     return 0;
// }
