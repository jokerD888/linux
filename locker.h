#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// ��װ�߳�ͬ�������࣬�ź����࣬�������࣬����������

// �ź�����
class sem {
public:
    // init,��������ʼ���ź���
    sem() {
        // �ڶ�������Ϊ0��������ź����ǵ�ǰ���̵ľֲ��ź����������ڶ�����̼乲��
        if (sem_init(&m_sem, 0, 0)!=0) {    
            // ���캯���޷���ֵ��ͨ���׳��쳣����������
            throw std::exception();
        }
    }
    // �����ź���
    ~sem() {
        sem_destroy(&m_sem);
    }
    // �ȴ��ź���
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }
    // �����ź���
    bool post() {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

// ��������
class locker {
public:
    // ��������ʼ��������
    locker() {
        if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
            throw std::exception();
        }
    }
    // ���ٻ�����
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }
    // ��ȡ������
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    // �ͷŻ�����
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    
private:
    pthread_mutex_t m_mutex;
};

// ����������
class cond {
public:
    // ��������ʼ����������
    cond() {
        if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
            throw std::exception();
        }
        if (pthread_cond_init(&m_cond, nullptr) != 0) {
            // ���캯��һ�������⣬��Ӧ�������ͷ��Ѿ��ɹ������˵���Դ
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    // ������������
    ~cond() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    // �ȴ���������
    bool wait() {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);   // waitǰ����Ҫ������
        ret = pthread_cond_wait(&m_cond, &m_mutex); //�ɹ�����0
        pthread_mutex_unlock(&m_mutex);  // ����
        return ret == 0;        
    }
    // ���ѵȴ������������߳�
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif