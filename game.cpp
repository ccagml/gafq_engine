#include <boost/program_options.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
// #include <boost/bind/bind.hpp>
#include <string>
#include <iostream>

#include <game_config.h>

//mysql
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
  try
  {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("props", boost::program_options::value<std::string>(), "props");

    boost::program_options::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (not vm.count("props"))
    {
      std::cerr << "props not exists." << std::endl;
      return -1;
    }

    std::string props = vm["props"].as<std::string>();
    if (!boost::filesystem::exists(props))
    {
      std::cerr << props << " not exists." << std::endl;
      return -1;
    }
    GameConfig::get_instance()->load_config(props);
    std::cout << "0:name:" << GameConfig::get_instance()->get_config_string("name") << std::endl;
    std::cout << "0:object.currency:" << GameConfig::get_instance()->get_config_string("object.currency") << std::endl;
    std::cout << "1:pi:" << GameConfig::get_instance()->get_config_float("pi") << std::endl;

    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
    {
      fprintf(stderr, "mysql_init() failed\n");
      exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", "shop1_kengqie_c", "aFhNbrjMCH",
                           "shop1_kengqie_c", 0, NULL, 0) == NULL)
    {
    }

    if (mysql_query(con, "SELECT * FROM dbshop_goods_class"))
    {
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL)
    {
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)))
    {
      for (int i = 0; i < num_fields; i++)
      {
        printf("%s ", row[i] ? row[i] : "NULL");
      }

      printf("\n");
    }

    mysql_free_result(result);
    mysql_close(con);

    return 0;
  }
  catch (boost::filesystem::filesystem_error &ex)
  {
    std::cerr << ex.code().message() << '\n';
  }
}