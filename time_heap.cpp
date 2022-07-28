
#ifndef intIME_HEAP
#define intIME_HEAP

#include <iostream>
#include <netinet/in.h>
#include <time.h>
using std::exception;   // 标准异常类

#define BUFFER_SIZE 64  

class heap_timer;       // 前置声明
struct client_data  //绑定socket和定时器
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer* timer;
};

// 定时器类
class heap_timer
{
public:
    heap_timer(int delay)
    {
        expire = time(NULL) + delay;
    }

public:
    time_t expire;                  // 定时器生效的绝对时间
    void (*cb_func)(client_data*);  // 定时器的回调函数
    client_data* user_data;         // 用户数据
};

// 时间堆类
class time_heap
{
public:
    // 构造函数之一，初始化一个大小为cap的空堆
    time_heap(int cap) throw (std::exception)   // 程序可能会抛excepton类型的函数
        : capacity(cap), cur_size(0)
    {
        array = new heap_timer * [capacity];
        if (!array)
        {
            throw std::exception();
        }
        for (int i = 0; i < capacity; ++i)
        {
            array[i] = nullptr;
        }
    }
    // 构造函数之二，用已有数组初始化堆
    time_heap(heap_timer** init_array, int size, int capacity) throw (std::exception)
        : cur_size(size), capacity(capacity)
    {
        if (capacity < size)
        {
            throw std::exception();
        }
        array = new heap_timer * [capacity];
        if (!array)
        {
            throw std::exception();
        }
        for (int i = 0; i < capacity; ++i)
        {
            array[i] = nullptr;
        }
        if (size != 0)
        {
            for (int i = 0; i < size; ++i)
            {
                array[i] = init_array[i];
            }
            // 对数组中的第[(cur_size-2)/2]~0个元素执行向下调整操作
            // 倒着从最后一颗子树（即最后一个非叶子节点）开始调整
            for (int i = (cur_size - 2)>>1; i >= 0; --i)
            {
                percolate_down(i);
            }
        }
    }
    // 销毁时间堆
    ~time_heap()
    {
        for (int i = 0; i < cur_size; ++i)
        {
            delete array[i];
        }
        delete[] array;
    }

public:
    // 添加目标定时器timer
    void add_timer(heap_timer* timer) throw (std::exception)
    {
        if (!timer)
        {
            return;
        }
        if (cur_size >= capacity)   //空间不够了，扩容
        {
            resize();
        }
        // 新插入元素，当前堆大小加1，hole是新建空穴的位置
        int hole = cur_size++;      
        int parent = 0;
        // 对空穴到根节点的路径上的所有节点执行上滤操作
        for (; hole > 0; hole = parent)
        {
            parent = (hole - 1) / 2;    // 父节点
            if (array[parent]->expire <= timer->expire) // 如果父节点已经小于等于孩子了，break;
            {
                break;
            }
            array[hole] = array[parent];                // 交换位置
        }
        array[hole] = timer;        // 放入合适的位置
    }
    // 删除目标定时器
    void del_timer(heap_timer* timer)
    {
        if (!timer)
        {
            return;
        }
        // lazy delelte
        // 仅仅将目标定时器的回调函数设为空，等到期了自动销毁，即所谓的延迟销毁。这将节省真正删除该定时器造成的开销，但这样做容易使堆数组膨胀
        timer->cb_func = nullptr;
    }
    // 获取堆顶部定时器
    heap_timer* top() const
    {
        if (empty())
        {
            return NULL;
        }
        return array[0];
    }
    // 删除堆顶部定时器
    void pop_timer()
    {
        if (empty())
        {
            return;
        }
        if (array[0])
        {
            delete array[0];    // 释放
            array[0] = array[--cur_size];   // 将数组最后一个元素放于堆顶，然后执行下滤操作
            percolate_down(0);
        }
    }
    // 心博函数
    void tick()
    {
        heap_timer* tmp = array[0];
        time_t cur = time(NULL);
        // 循环处理堆中到期定时器
        while (!empty())
        {
            if (!tmp)
            {
                break;
            }
            // 如果堆顶定时器没到期，break
            if (tmp->expire > cur)
            {
                break;
            }
            // 否则就执行堆中定时器中的任务
            if (array[0]->cb_func)  // 如果任务不为空的话
            {
                array[0]->cb_func(array[0]->user_data);
            }
            pop_timer();    // 弹出堆顶元素
            tmp = array[0]; // 取得新的堆顶元素
        }
    }
    // 判断堆是否为空
    bool empty() const { return cur_size == 0; }

private:
    // 下滤算法（堆的向下调整算法），确保堆数组中第hole个节点作为根的子树拥有最小堆的性质
    // 时间O(logN)
    void percolate_down(int hole)
    {
        heap_timer* temp = array[hole];
        int child = 0;
        for (; ((hole * 2 + 1) <= (cur_size - 1)); hole = child)
        {
            child = hole * 2 + 1;   // 左孩子
            // 有右孩子的话，且 右孩子的值大于左孩子，那么指向右孩子
            if ((child < (cur_size - 1)) && (array[child + 1]->expire < array[child]->expire))  
            {
                ++child;    // 修改指向
            }
            if (array[child]->expire < temp->expire)    // 最小的孩子小于父亲的话，交换位置
            {
                array[hole] = array[child];             
            } else                      // 否则，符合堆的要求，break;
            {
                break;
            }
        }
        array[hole] = temp; // hole 来到了其该去的位置
    }
    // 扩容，扩容因子为2
    void resize() throw (std::exception)
    {
        heap_timer** temp = new heap_timer * [2 * capacity];
        for (int i = 0; i < 2 * capacity; ++i)
        {
            temp[i] = nullptr;
        }
        if (!temp)
        {
            throw std::exception();
        }
        capacity = 2 * capacity;    
        // 将原数组元素（指针）复制回去
        for (int i = 0; i < cur_size; ++i)
        {
            temp[i] = array[i];
        }
        // 释放原数组
        delete[] array;
        array = temp;   // 修改指向
    }

private:
    heap_timer** array;
    int capacity;   // 容量
    int cur_size;   // 当前大小
};

#endif
