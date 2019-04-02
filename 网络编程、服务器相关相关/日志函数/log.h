#ifndef LOG_H  
#define LOG_H  
  
#include <stdio.h>  
#include <iostream>  
#include <string>  
#include <stdarg.h>  
#include <pthread.h>  
#include "block_queue.h"  
using namespace std;  
  
class Log  
{  
    public:  
	//单例模式，保证仅有一个
        static Log* get_instance()  
        {  
            static Log instance;  
            return &instance;  
        }  
     //异步写
        static void *flush_log_thread(void* args)  
        {  
            Log::get_instance()->async_write_log();  
        }  
  //初始化
        bool init(const char* file_name, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);  
  //写日志
        void write_log(int level, const char* format, ...);  
  //刷新
        void flush(void);  
  
    private:  
        Log();  
        virtual ~Log();  
        void *async_write_log()  
        {  
            string single_log;  
            while(m_log_queue->pop(single_log))  
            {  
                pthread_mutex_lock(m_mutex);  
                fputs(single_log.c_str(), m_fp);  
                pthread_mutex_unlock(m_mutex);  
            }  
        }  
  
    private:  
        pthread_mutex_t *m_mutex;  
        char dir_name[128];  
        char log_name[128];  
        int m_split_lines;  //切割行数
        int m_log_buf_size;  //日志缓冲区大小
        long long  m_count;  
        int m_today;  //保存天
        FILE *m_fp;  //打开的文件指针
        char *m_buf;  //缓冲区指针
        block_queue<string> *m_log_queue;  //采用写的可重入队列（安全）保存要写入日志。
        bool m_is_async;  //是否异步写
};  
  
#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, format, __VA_ARGS__)  
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, format, __VA_ARGS__)  
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, format, __VA_ARGS__)  
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, format, __VA_ARGS__)  
  
#endif  