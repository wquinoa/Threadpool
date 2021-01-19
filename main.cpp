#include <iostream>
#include <queue>
#include <deque>
#include "threadpool.hpp"
#include <signal.h>

#define WORKERS 8
#define MAX_QUEUE 240

pthread_mutex_t writeLock;

static int _index;

class TestClass
{
    int n;
    int i;
 public:

    TestClass() : n(0) { i = ++_index; };

    void    increment() { ++n; }

    int const &getNum() const { return n; };
    int const &getIndex() const { return i; };
};

/* Derived to define the actual jobs */

template<typename T>
class ThreadpoolDerived : public Threadpool<T>
{

 public:

    ThreadpoolDerived(size_t w, size_t q) : Threadpool<T>(w, q) {};

    void job(TestClass *task)
    {
        int i;

        task->increment();
        i = task->getNum();
        if (i % 1000 == 0)
        {
            pthread_mutex_lock(&writeLock);
            std::cout << "Task " << task->getIndex() << ": " << i << std::endl;
            pthread_mutex_unlock(&writeLock);
        }
    }
};

size_t getResult(TestClass *task, int workers)
{
    size_t result = 0;

    for (int i = 0; i < workers; ++i)
        result += task[i].getNum();
    return(result);
}

int main()
{
    TestClass tab[WORKERS];
    ThreadpoolDerived<TestClass> q(WORKERS, MAX_QUEUE);
    size_t target;

    pthread_mutex_init(&writeLock, NULL);

    target = MAX_QUEUE * 100000;
    for (size_t i = 0; i < (target); ++i)
        q.addTask(&tab[ i % WORKERS ]);

    while (getResult(tab, WORKERS) < target)
        usleep(10000);

    std::cout << getResult(tab, WORKERS) << " tasks completed" << std::endl;
    pthread_mutex_destroy(&writeLock);
}
