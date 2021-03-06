//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "socket_msg.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>

using boost::asio::ip::tcp;

typedef std::deque<socket_msg> socket_msg_queue;

class chat_client
{
public:
    chat_client(boost::asio::io_service &io_service, tcp::resolver::iterator endpoint_iterator) : io_service_(io_service), socket_(io_service)
    {
        boost::asio::async_connect(socket_, endpoint_iterator, boost::bind(&chat_client::handle_connect, this, boost::asio::placeholders::error));
    }

    void write(const socket_msg &msg) { io_service_.post(boost::bind(&chat_client::do_write, this, msg)); }

    void close() { io_service_.post(boost::bind(&chat_client::do_close, this)); }

private:
    void handle_connect(const boost::system::error_code &error)
    {
        if (!error)
        {
            boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.data(), socket_msg::header_length), boost::bind(&chat_client::handle_read_header, this, boost::asio::placeholders::error));
        }
    }

    void handle_read_header(const boost::system::error_code &error)
    {
        if (!error && read_msg_.decode_header())
        {
            boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.body(), read_msg_.body_length()), boost::bind(&chat_client::handle_read_body, this, boost::asio::placeholders::error));
        }
        else
        {
            do_close();
        }
    }

    void handle_read_body(const boost::system::error_code &error)
    {
        if (!error)
        {
            std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << "\n";
            boost::asio::async_read(socket_, boost::asio::buffer(read_msg_.data(), socket_msg::header_length), boost::bind(&chat_client::handle_read_header, this, boost::asio::placeholders::error));
        }
        else
        {
            do_close();
        }
    }

    void do_write(socket_msg msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                                     boost::bind(&chat_client::handle_write, this, boost::asio::placeholders::error));
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
                                         boost::bind(&chat_client::handle_write, this, boost::asio::placeholders::error));
            }
        }
        else
        {
            do_close();
        }
    }

    void do_close() { socket_.close(); }

private:
    boost::asio::io_service &io_service_;
    tcp::socket socket_;
    socket_msg read_msg_;
    socket_msg_queue write_msgs_;
};

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: chat_client <host> <port>\n";
        return 1;
    }

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], argv[2]);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    chat_client c(io_service, iterator);

    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
    int cnt_n = atoi(argv[3]);

    char line[socket_msg::max_body_length + 1];
    while (std::cin.getline(line, socket_msg::max_body_length + 1))
    {
        //{"action":"foo","data":"{\"from\":1,\"to\":2,\"info\":\"sss\"}"}
        using namespace std; // For strlen and memcpy.
        socket_msg msg;
        msg.body_length(strlen(line));
        memcpy(msg.body(), line, msg.body_length());
        msg.encode_header();
        for (int i = 0; i < cnt_n; i++)
        {
            c.write(msg);
        }
    }

    c.close();
    t.join();

    return 0;
}