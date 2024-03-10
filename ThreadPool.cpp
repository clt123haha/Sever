#include <iostream>
#include <queue>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

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

    void enqueue(void (*function)())
    {
        printf("push\n");

        pthread_mutex_lock(&mutex);
        tasks.push(function);
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
    queue<void (*)()> tasks;

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
            void (*task)() = pool->tasks.front();
            pool->tasks.pop();
            pthread_mutex_unlock(&(pool->mutex));

            task();
        }
    }
};

void taskFunction() {
    cout << "Down!!!!!" << std::endl;
    sleep(1);
}

int main()
{
    ThreadPool* pool = new ThreadPool(4);

    for (int i = 0; i < 10; i++)
    {
        pool->enqueue(taskFunction);
    }

    sleep(5);

    return 0;
}