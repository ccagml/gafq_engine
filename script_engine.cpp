// Using gafq as a C library
extern "C"
{
#include <gafq.h>
#include <gafqlib.h>
#include <gauxlib.h>
}

#include "script_engine.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

ScriptEngine *ScriptEngine::script_eng = nullptr;

bool ScriptEngine::Init(std::string init_file)
{
    std::cout << "Init" << std::endl;
    this->init_file = init_file;
    this->se_gafqState = gafqL_newstate();
    gafqL_openlibs(this->se_gafqState);
    // gafqL_loadfile(this->se_gafqState, this->init_file.c_str());
    // luaL_loadfile just loads the file, it does not run it. Try luaL_dofile instead.
    gafqL_dofile(this->se_gafqState, this->init_file.c_str());
    gafq_newtable(this->se_gafqState);
    gafq_pcall(this->se_gafqState, 0, 0, 0);
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

// 循环执行
void ScriptEngine::LoopExecute()
{
    std::vector<GAFQITEM_PTR> temp_execute_list;
    mtx_.lock();
    int n = this->wait_execute_list.size();
    for (int i = 0; i < n; i++)
    {
        temp_execute_list.push_back(this->wait_execute_list[i]);
    }
    this->wait_execute_list.clear();
    mtx_.unlock();
    //执行
    int tn = temp_execute_list.size();
    for (int i = 0; i < tn; i++)
    {
        std::string action = temp_execute_list[i]->msg->action;
        std::string data = temp_execute_list[i]->msg->data;
        int fd = temp_execute_list[i]->fd;

        // load the function from global
        gafq_getglobal(this->se_gafqState, action.c_str());
        if (gafq_isfunction(this->se_gafqState, -1))
        {
            // push function arguments into stack
            gafq_pushnumber(this->se_gafqState, fd);
            gafq_pushstring(this->se_gafqState, data.c_str());
            gafq_pcall(this->se_gafqState, 2, 0, 0);
            if (!gafq_isnil(this->se_gafqState, -1))
            {
                fprintf(stderr, "Failed to run script: %s\n", gafq_tostring(this->se_gafqState, -1));
                gafq_pop(this->se_gafqState, 1);
            }
        }
        else
        {
            fprintf(stderr, "不是函数 script: %s\n", gafq_tostring(this->se_gafqState, -1));
        }

        // gafq_setglobal(this->se_gafqState, "foo");
        // gafq_pushnumber(this->se_gafqState, fd);
        // gafq_pushstring(this->se_gafqState, "dataaaa");
        // int call_result = gafq_pcall(this->se_gafqState, 2, 0, 0);
        // if (call_result)
        // {
        //     fprintf(stderr, "Failed to run script: %s\n", gafq_tostring(this->se_gafqState, -1));
        //     gafq_pop(this->se_gafqState, 1);
        // }
    }
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

int main(int argc, char const *argv[])
{
    /* code */

    ScriptEngine::get_instance()->Init("script.gafq");
    while (true)
    {
        std::shared_ptr<ScriptEngineMsgBase> foo = std::make_shared<ScriptEngineMsgBase>();
        std::cout << "action:" << std::endl;
        std::cin >> foo->action;
        std::cout << "data:" << std::endl;
        std::cin >> foo->data;
        ScriptEngine::get_instance()->ExecGafq(std::string(), -1, 1, foo, 1, 1, std::string());
        ScriptEngine::get_instance()->LoopExecute();
    }
    // boost::thread_group threads;
    // for (int i = 0; i < 30; ++i)
    // {
    //     threads.create_thread(&increment_count);
    // }
    // threads.join_all();
    // threads.wait();
    // std::cout << "上锁长度" << ScriptEngine::get_instance()->wait_execute_list.size() << ": count" << ScriptEngine::get_instance()->count << ": count2:" << ScriptEngine::get_instance()->count2
    //           << std::endl;
    return 0;
}
