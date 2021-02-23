#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
// #include <boost/bind/bind.hpp>
#include <game_config.h>

#include <iostream>
#include <string>

// mysql
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

// thread
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "socket_msg.h"

bool init_load_config(int argc, const char *argv[])
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("props", boost::program_options::value<std::string>(), "props");

    boost::program_options::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (not vm.count("props"))
    {
        std::cerr << "props not exists." << std::endl;
        return false;
    }

    std::string props = vm["props"].as<std::string>();
    if (!boost::filesystem::exists(props))
    {
        std::cerr << props << " not exists." << std::endl;
        return false;
    }
    GameConfig::get_instance()->load_config(props);
    return true;
}

bool init_db()
{

    std::cout << "init_db线程id:" << boost::this_thread::get_id() << std::endl;
    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
    {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", "shop_kengqie_co", "BShBje4HYA", "shop_kengqie_co", 0, NULL, 0) == NULL)
    {
    }

    if (mysql_query(con, "SELECT * FROM dbshop_ad"))
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
    return true;
}

typedef std::deque<socket_msg> socket_msg_queue;

class socket_session : public boost::enable_shared_from_this<socket_session>
{
public:
    socket_session(boost::asio::io_service &io_service) : socket_(io_service) {}

    boost::asio::ip::tcp::socket &socket() { return socket_; }

    void start()
    {
        boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.data(), socket_msg::header_length),
                                boost::bind(&socket_session::handle_read_header, shared_from_this(), boost::asio::placeholders::error));
    }

    void deliver(const socket_msg &msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                                     boost::bind(&socket_session::handle_write, shared_from_this(), boost::asio::placeholders::error));
        }
    }

    void handle_read_header(const boost::system::error_code &error)
    {
        if (!error && read_msg_.decode_header())
        {
            boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                    boost::bind(&socket_session::handle_read_body, shared_from_this(), boost::asio::placeholders::error));
        }
    }

    void handle_read_body(const boost::system::error_code &error)
    {
        if (!error)
        {
            std::cout << "Message: " << read_msg_.data() << std::endl;
            boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.data(), socket_msg::header_length),
                                    boost::bind(&socket_session::handle_read_header, shared_from_this(), boost::asio::placeholders::error));
        }
    }

    void handle_write(const boost::system::error_code &error)
    {
        if (!error)
        {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
                boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                                         boost::bind(&socket_session::handle_write, shared_from_this(), boost::asio::placeholders::error));
            }
        }
    }

private:
    boost::asio::ip::tcp::socket socket_;
    socket_msg read_msg_;
    socket_msg_queue write_msgs_;
};

typedef boost::shared_ptr<socket_session> socket_session_ptr;

//----------------------------------------------------------------------

class socket_server
{
public:
    socket_server(boost::asio::io_service &io_service, const boost::asio::ip::tcp::endpoint &endpoint) : io_service_(io_service), acceptor_(io_service, endpoint) { start_accept(); }

    void start_accept()
    {
        socket_session_ptr new_session(new socket_session(io_service_));
        acceptor_.async_accept(new_session->socket(), boost::bind(&socket_server::handle_accept, this, new_session, boost::asio::placeholders::error));
    }

    void handle_accept(socket_session_ptr session, const boost::system::error_code &error)
    {
        if (!error)
        {
            session->start();
        }

        start_accept();
    }

private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

typedef boost::shared_ptr<socket_server> socket_server_ptr;
typedef std::list<socket_server_ptr> socket_server_list;

//----------------------------------------------------------------------

// 初始化监听
bool init_server()
{
    std::cout << "init_server线程id:" << boost::this_thread::get_id() << std::endl;
    std::cout << "init_server 1" << std::endl;
    boost::asio::io_service socket_io_service;
    std::cout << "init_server 2" << std::endl;
    std::list<socket_server> servers;
    std::cout << "init_server 3" << std::endl;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 6000);
    std::cout << "init_server 4" << std::endl;
    servers.emplace_back(socket_io_service, endpoint);
    std::cout << "init_server 5" << std::endl;
    socket_io_service.run();
    return true;
}

int main(int argc, const char *argv[])
{
    try
    {
        init_load_config(argc, argv);
        std::cout << "0:name:" << GameConfig::get_instance()->get_config_string("name") << std::endl;
        std::cout << "0:object.currency:" << GameConfig::get_instance()->get_config_string("object.currency") << std::endl;
        std::cout << "1:pi:" << GameConfig::get_instance()->get_config_float("pi") << std::endl;
        // init_db();
        // init_server();

        boost::thread_group threads;
        threads.create_thread(&init_server);
        // threads.create_thread(&init_db);
        threads.join_all();

        while (true)
        {

            boost::this_thread::sleep(boost::posix_time::seconds(1));
            boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

            std::cout << "线程id:" << boost::this_thread::get_id() << std::endl;
        }
        return 0;
    }
    catch (boost::filesystem::filesystem_error &ex)
    {
        std::cerr << ex.code().message() << '\n';
    }
}