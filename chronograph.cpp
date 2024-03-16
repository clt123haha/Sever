#include <iostream>
#include <queue>
#include <pthread.h>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/time.h>

using namespace std;


class TimerNode {
public:
    TimerNode(int timeout) : deleted_(false)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        // 以毫秒计
        expiredTime_ =
            (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
    }

    ~TimerNode()
    {
        //对绑定的函数做出相关操作
    }

    void update(int timeout)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        // 以毫秒计
        expiredTime_ =
            (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
    }

    bool isValid()
    {
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
            if (temp < expiredTime_)
                return true;
            else {
                this->setDeleted();
                return false;
            }
        }
    }

    void clearReq()
    {
        //任务状态重置


        this->setDeleted();
    }

    void setDeleted() { deleted_ = true; }
    bool isDeleted() const { return deleted_; }
    size_t getExpTime() const { return expiredTime_; }

private:
    bool deleted_;
    size_t expiredTime_;
};


class Task
{
public:
    Task(void (*function)(void*), int timeout) :function(function), time(timeout),timeout(timeout)
    {

    }

    bool isTimeOut()
    {
        return !time.isValid();
    }

    void update()
    {
        time.update(timeout);
    }

    void update(int timeout)
    {
        this->timeout = timeout;
        time.update(timeout);
    }

    void work(void * arg)
    {
        function(arg);
    }
private:
    int timeout;
    void (*function)(void*);
    TimerNode time;
};


class ThreadPool
{
public:
    ThreadPool(int num)
    {
        mutex = PTHREAD_MUTEX_INITIALIZER;
        condition = PTHREAD_COND_INITIALIZER;

        pthread_mutex_init(&mutex, NULL);

        for (int i = 0; i < num; i++)
        {
            pthread_t pid;
            int err = pthread_create(&pid, NULL, ThreadPool::work, this);
            if (err != 0)
            {
                printf("创建工作线程时出现问题");
                return;
            }
        }
    }

    void enqueue(Task function, void* arg)
    {
        pthread_mutex_lock(&mutex);
        tasks.push(function);
        taskArgs.push(arg);
        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&condition);
    }

    ~ThreadPool() {
        for (int i = 0; i < workers.size(); ++i) {
            pthread_join(workers[i], NULL);
        }
    }

private:
    vector<pthread_t> workers;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    queue<Task> tasks;
    queue<void*> taskArgs;

    static void* work(void* arg)
    {
        ThreadPool* pool = (ThreadPool*)arg;
        while (true)
        {
            pthread_mutex_lock(&(pool->mutex));
            while (pool->tasks.empty())
            {
                pthread_cond_wait(&(pool->condition), &(pool->mutex));
            }
            Task task = pool->tasks.front();
            void* arg = pool->taskArgs.front();
            pool->tasks.pop();
            pool->taskArgs.pop();
            pthread_mutex_unlock(&(pool->mutex));

            task.update();
            task.work(arg);

            if (task.isTimeOut())
            {
                cout << "Time Out" << endl;
                
                //加入重做
                /*pthread_mutex_lock(&(pool->mutex));
                pool->tasks.push(task);
                pool->taskArgs.push(arg);
                pthread_mutex_unlock(&(pool->mutex));*/
            }
                
        }
    }
};

void task1(void* arg)
{
    sleep(2);

   cout << "task1" << endl;
}

void task2(void* arg)
{
    cout << "task2" << endl;
}

int main()
{
    ThreadPool pool(3);

    Task test1(task1, 2);
    Task test2(task2, 2);

    pool.enqueue(test1, nullptr);

    pool.enqueue(test2, nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(3)); // Allow tasks to complete

    return 0;
}
