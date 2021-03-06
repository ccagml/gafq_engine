#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H
extern "C"
{
#include <gafq.h>
#include <gafqlib.h>
#include <gauxlib.h>
}
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <string>
#include <vector>

#include <script_engine_msg_base.h>
class ScriptEngine
{

    boost::mutex mtx_; // 现成锁
    // 1、定义了一个新的结构类型gafqItem
    // 2、使用 typedef 为这个新的结构起了一个别名，叫 GAFQITEM
    typedef struct gafqItem
    {
        std::string gafq_content;
        int fd;
        std::shared_ptr<ScriptEngineMsgBase> msg;
        std::string errinfo;
        int status;
    } GAFQITEM;
    typedef boost::shared_ptr<GAFQITEM> GAFQITEM_PTR;

public:
    // 构造函数
    ScriptEngine() {}
    ~ScriptEngine() {}
    static ScriptEngine *script_eng;
    int count;
    int count2;
    static ScriptEngine *get_instance()
    {
        if (script_eng == nullptr)
        {
            script_eng = new ScriptEngine();
            script_eng->count = 0;
            script_eng->count2 = 0;
        }
        return script_eng;
    }

    gafq_State *get_gafq_State() { return this->se_gafqState; }
    bool Init(std::string init_file);
    void ExecGafq(std::string gafq_content = std::string(), int fd = -1, std::shared_ptr<ScriptEngineMsgBase> msg = nullptr, int status = 1, std::string errinfo = std::string());
    void LoopExecute(); // 循环执行
    std::vector<GAFQITEM_PTR> wait_execute_list;

private:
    gafq_State *se_gafqState;
    std::string init_file;
};

#endif