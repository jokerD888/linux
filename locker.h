#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 封装线程同步机制类，信号量类，互斥锁类，条件变量类

// 信号量类
class sem {
public:
    // init,创建并初始化信号量
    sem() {
        // 第二个参数为0，表这个信号量是当前进程的局部信号量，不可在多个进程间共享
        if (sem_init(&m_sem, 0, 0)!=0) {    
            // 构造函数无返回值，通过抛出异常来报考错误
            throw std::exception();
        }
    }
    // 销毁信号量
    ~sem() {
        sem_destroy(&m_sem);
    }
    // 等待信号量
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }
    // 增加信号量
    bool post() {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

// 互斥锁类
class locker {
public:
    // 创建并初始化互斥锁
    locker() {
        if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
            throw std::exception();
        }
    }
    // 销毁互斥锁
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }
    // 获取互斥锁
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    // 释放互斥锁
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    
private:
    pthread_mutex_t m_mutex;
};

// 条件变量类
class cond {
public:
    // 创建并初始化条件变量
    cond() {
        if (pthread_cond_init(&m_cond, nullptr) != 0) {
            // 构造函数一旦出问题，就应该立即释放已经成功分配了的资源
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    // 销毁条件变量
    ~cond() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    // 等待条件变量
    bool wait() {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);   // wait前，需要先上锁
        ret = pthread_cond_wait(&m_cond, &m_mutex); //成功返回0
        pthread_mutex_unlock(&m_mutex);  // 解锁
        return ret == 0;        
    }
    // 唤醒等待条件变量的线程
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif
