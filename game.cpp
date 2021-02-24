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

#include <thread>

#include "socket_msg.h"

#include <sys/epoll.h>
#include <sys/socket.h>

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

    std::cout << "init_db线程id:" << std::this_thread::get_id() << std::endl;
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

void init_boost_asio()
{
    std::cout << "init_boost_asio线程id:" << std::this_thread::get_id() << std::endl;
    boost::asio::io_service socket_io_service;
    std::list<socket_server> servers;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 6000);
    servers.emplace_back(socket_io_service, endpoint);
    socket_io_service.run();
}

#define BUF_SIZE 100
#define EPOLL_SIZE 1000

void init_epool()
{
    // Step 1. Initialization
    int server_socket, client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    int str_len, i;
    char buf[BUF_SIZE];

    int epfd, event_cnt;

    // Step 2. Creating a socket
    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(6000);

    // Step 3. Binding the server address onto the socket created just right before.
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cout << "bind() error" << std::endl;
        return;
    }

    // Step 4. Start to listen to the socket.
    if (listen(server_socket, 6000) == -1)
    {
        std::cout << "listen() error" << std::endl;
        return;
    }

    // Step 5. Create an event poll instance.
    epfd = epoll_create(255);
    auto epoll_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_socket;

    // Step 6. Adding the server socket file descriptor to the event poll's control.
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &event);
    int recv_cnt = 0;
    socket_msg read_msg_;
    while (true)
    {
        // Step 7. Wait until some event happens
        event_cnt = epoll_wait(epfd, epoll_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            std::cout << "epoll_wait() error" << std::endl;
            break;
        }

        for (i = 0; i < event_cnt; i++)
        {
            if (epoll_events[i].data.fd == server_socket)
            {
                std::cout << "data.fd == server_socket" << std::endl;
                addr_size = sizeof(client_addr);
                client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
                event.events = EPOLLIN;
                event.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);
                // printf("Connected client: %d\n", client_socket);
            }
            else if (epoll_events[i].events & EPOLLIN)
            {
                // std::cout << " EPOLLIN fd:" << epoll_events[i].data.fd << std::endl;
                str_len = read(epoll_events[i].data.fd, read_msg_.data(), socket_msg::header_length);
                std::cout << " EPOLLIN fd str_len:" << str_len << std::endl;
                if (str_len > 0 && read_msg_.decode_header())
                {
                    int str_len1 = read(epoll_events[i].data.fd, read_msg_.body(), read_msg_.body_length());
                    std::cout << epoll_events[i].data.fd << "Message: " << str_len << ":" << str_len1 << " buf:" << read_msg_.data() << std::endl;
                }
                else
                {
                    close(epoll_events[i].data.fd);
                }
                // memset(buf, 0, sizeof(buf));
                // str_len = read(epoll_events[i].data.fd, buf, 16);
                // if (str_len > 0)
                // {
                //     std::cout << epoll_events[i].data.fd << ":send Message:" << str_len << "buf:" << buf << std::endl;
                // }
                // else
                // {
                //     close(epoll_events[i].data.fd);
                // }
            }
            else if (epoll_events[i].events & EPOLLERR)
            {
                std::cout << " EPOLLERR fd:" << epoll_events[i].data.fd << std::endl;
                close(epoll_events[i].data.fd);
            }
            else // client socket?
            {
                // if (str_len == 0) // close request!
                // {
                //     epoll_ctl(epfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, nullptr);
                //     close(epoll_events[i].data.fd);
                //     printf("%d: %s\n", ++recv_cnt, buf);
                //     // printf("closed client: %d \n", epoll_events[i].data.fd);
                // }
                // else
                // {
                //     write(epoll_events[i].data.fd, buf, str_len); // echo!
                // }
            } // end of else()
        }     // end of for()
    }         // end of while()

    close(server_socket);
    close(epfd);
    free(epoll_events);
}

// 初始化监听
bool init_server()
{
    std::cout << "init_server线程id:" << std::this_thread::get_id() << std::endl;
    std::cout << "init_server 1" << std::endl;
    // init_boost_asio();
    init_epool();
    return true;
}

int main(int argc, const char *argv[])
{
    try
    {
        init_load_config(argc, argv);
        std::cout << "主线程id:" << std::this_thread::get_id() << std::endl;
        std::cout << "0:name:" << GameConfig::get_instance()->get_config_string("name") << std::endl;
        std::cout << "0:object.currency:" << GameConfig::get_instance()->get_config_string("object.currency") << std::endl;
        std::cout << "1:pi:" << GameConfig::get_instance()->get_config_float("pi") << std::endl;
        // init_db();
        // init_server();

        // boost::thread_group threads;
        // threads.create_thread(&init_server);
        // // threads.create_thread(&init_db);
        // threads.join_all();

        std::thread{&init_server}.detach();

        std::cout << "主线程id:" << std::this_thread::get_id() << std::endl;
        while (true)
        {

            // std::this_thread::sleep(boost::posix_time::seconds(1));
            // std::this_thread::sleep(boost::posix_time::milliseconds(1000));
            std::chrono::milliseconds dura(1000);
            std::this_thread::sleep_for(dura);

            // std::cout << "主线程id:" << std::this_thread::get_id() << std::endl;
        }
        return 0;
    }
    catch (boost::filesystem::filesystem_error &ex)
    {
        std::cerr << ex.code().message() << '\n';
    }
}