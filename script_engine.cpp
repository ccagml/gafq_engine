#include "script_engine.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

ScriptEngine *ScriptEngine::script_eng = nullptr;

bool ScriptEngine::Init(std::string init_file)
{
    std::cout << "Init" << std::endl;
    this->init_file = init_file;
    return true;
};

void ScriptEngine::ExecGafq(std::string lua_content, int fd, int func, std::shared_ptr<ScriptEngineMsgBase> msg, int cookie, int ret, std::string errmsg)
{

    GAFQITEM_PTR gt_ptr = boost::make_shared<GAFQITEM>();
    // 将该操作放入队列中，在下次循环时取出执行

    if (!lua_content.empty())
    {
        gt_ptr->lua_content = lua_content;
        gt_ptr->func = -1;
        gt_ptr->fd = fd;
        gt_ptr->msg = nullptr;
        gt_ptr->cookie = -1;
        gt_ptr->ret = -1;
    }
    else if (func != -1)
    {
        gt_ptr->func = func;
        gt_ptr->fd = fd;
        gt_ptr->msg = msg;
        gt_ptr->errmsg = errmsg;
        gt_ptr->cookie = cookie;
        gt_ptr->ret = ret;
    }
    else
    {
        return;
    }

    // 放入队列中
    mtx_.lock();
    wait_execute_list.push_back(gt_ptr);

    // std::cout<<"上锁长度"<<wait_execute_list.size()<<std::endl;
    count2 = count2 + 1;
    mtx_.unlock();
    count = count + 1;
}
// void increment_count()
// {
//     // boost::unique_lock<boost::mutex> lock(mutex);
//     for(int i = 0; i < 10000; i++){
//         // count = count + 1;
//         ScriptEngine::get_instance()->ExecGafq("print('1')");
//         //std::cout << "count = " << count << std::endl;
//     }
// }

// int main(int argc, char const *argv[])
// {
//     /* code */

//     ScriptEngine::get_instance()->Init("./config.json");
//     ScriptEngine::get_instance()->ExecGafq("print('1')");

//     boost::thread_group threads;
//     for (int i = 0; i < 30; ++i){
//         threads.create_thread(&increment_count);
//     }
    // threads.join_all();
    // // threads.wait();
//     std::cout << "上锁长度" << ScriptEngine::get_instance()->wait_execute_list.size() << ": count" << ScriptEngine::get_instance()->count << ": count2:" << ScriptEngine::get_instance()->count2 <<
//     std::endl; return 0;
// }
