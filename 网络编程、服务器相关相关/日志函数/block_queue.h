#ifndef BLOCK_QUEUE_H  
#define BLOCK_QUEUE_H  
  
#include <iostream>  
#include <stdlib.h>  
#include <pthread.h>  
#include <sys/time.h>  
using namespace std;  
  
template<class T>  
class block_queue  
{  
    public:  
        block_queue(int max_size = 1000)  //默认设置最大为1000
        {  
            if(max_size <= 0)  
            {  
                exit(-1);  
            }  
              
            m_max_size = max_size;  
            m_array = new T[max_size];  
            m_size = 0;  //队列初始大小为0
            m_front = -1;  //队头为-1；
            m_back = -1;  //队尾-1；
  
            m_mutex = new pthread_mutex_t;  //初始化互斥锁
            m_cond = new pthread_cond_t;  //初始化条件变量
            pthread_mutex_init(m_mutex, NULL);  
            pthread_cond_init(m_cond, NULL);  
        }  
  
        void clear()  
        {  
            pthread_mutex_lock(m_mutex);  
            m_size = 0;  
            m_front = -1;  
            m_back = -1;  
            pthread_mutex_unlock(m_mutex);  
        }  
  
        ~block_queue()  
        {  
		//由于基于多线程，保证线程安全，需要用锁保证释放安全
            pthread_mutex_lock(m_mutex);  
            if(m_array != NULL)  
                delete  m_array;  
            pthread_mutex_unlock(m_mutex);  
  
            pthread_mutex_destroy(m_mutex);  
            pthread_cond_destroy(m_cond);  
  
            delete m_mutex;  
            delete m_cond;  
        }  
      //判断队列是否满
        bool full()const  
        {  
            pthread_mutex_lock(m_mutex);  
            if(m_size >= m_max_size)  
            {  
                pthread_mutex_unlock(m_mutex);  
                return true;  
            }  
            pthread_mutex_unlock(m_mutex);  
            return false;  
        }  
    //判断队列是否为空
        bool empty()const  
        {  
            pthread_mutex_lock(m_mutex);  
            if(0 == m_size)  
            {  
                pthread_mutex_unlock(m_mutex);  
                return true;  
            }  
            pthread_mutex_unlock(m_mutex);  
            return false;  
        }  
     //取队头
        bool front(T& value)const  
        {  
            pthread_mutex_lock(m_mutex);  
            if(0 == m_size)  
            {  
                pthread_mutex_unlock(m_mutex);  
                return false;  
            }  
            value = m_array[m_front];  
            pthread_mutex_unlock(m_mutex);  
            return true;  
        }  
      //取队尾
        bool back(T& value)const  
        {  
            pthread_mutex_lock(m_mutex);  
            if(0 == m_size)  
            {  
                pthread_mutex_unlock(m_mutex);  
                return false;  
            }  
            value = m_array[m_back];  
            pthread_mutex_unlock(m_mutex);  
            return true;  
        }  
      //返回当前队列长度
        int size()const  
        {  
            int tmp = 0;  
            pthread_mutex_lock(m_mutex);  
            tmp = m_size;  
            pthread_mutex_unlock(m_mutex);  
            return tmp;  
        }  
     //返回队列最大长度
        int max_size()const  
        {  
            int tmp = 0;  
            pthread_mutex_lock(m_mutex);  
            tmp = m_max_size;  
            pthread_mutex_unlock(m_mutex);  
            return tmp;  
        }  
    //入队
        bool push(const T& item)  
        {  
            pthread_mutex_lock(m_mutex);  
            if(m_size >= m_max_size)  
            {  
                pthread_cond_broadcast(m_cond);  
                pthread_mutex_unlock(m_mutex);  
                return false;  
            }  
              
            m_back = (m_back + 1) % m_max_size;  
            m_array[m_back] = item;  
  
            m_size++;  
            pthread_cond_broadcast(m_cond);  
            pthread_mutex_unlock(m_mutex);  
  
            return true;  
        }  
    //出队
        bool pop(T& item)  
        {  
            pthread_mutex_lock(m_mutex);  
            while(m_size <= 0)  
            {  
                if(0 != pthread_cond_wait(m_cond, m_mutex))  
                {  
                    pthread_mutex_unlock(m_mutex);  
                    return false;  
                }  
            }  
  
            m_front = (m_front + 1) % m_max_size;  
            item = m_array[m_front];  
            m_size--;  
            pthread_mutex_unlock(m_mutex);  
            return true;  
        }  
   //按时间出队
        bool pop(T& item, int ms_timeout)  
        {  
            struct timespec t = {0,0};  
            struct timeval now = {0,0};  
            gettimeofday(&now, NULL);  
            pthread_mutex_lock(m_mutex);  
            if(m_size <= 0)  
            {  
                t.tv_sec = now.tv_sec + ms_timeout/1000;  
                t.tv_nsec = (ms_timeout % 1000)*1000;  
                if(0 != pthread_cond_timedwait(m_cond, m_mutex, &t))  
                {  
                    pthread_mutex_unlock(m_mutex);  
                    return false;  
                }  
            }  
  
            if(m_size <= 0)  
            {  
                pthread_mutex_unlock(m_mutex);  
                return false;  
            }  
  
            m_front = (m_front + 1) % m_max_size;  
            item = m_array[m_front];m_size--;  
            pthread_mutex_unlock(m_mutex);  
            return true;  
        }  
  
private:  
        pthread_mutex_t *m_mutex; //互斥锁 
        pthread_cond_t *m_cond; // 条件变量
        T *m_array;  //类型指针
        int m_size;  //队列长度
        int m_max_size;  //最长队列大小
        int m_front;  //队头
        int m_back;  //队尾
};  
  
#endif  
