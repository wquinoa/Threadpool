#include "threadpool.hpp"
#include <signal.h>

#define WORKERS 32U
#define QUEUE 100UL
#define QSIZE_FACTOR 10000UL
#define TOTAL_TASKS (QUEUE * QSIZE_FACTOR)
#define PRINT_FREQUENCY (QSIZE_FACTOR / 1)

pthread_mutex_t writeLock;

static int _index;

class TestClass
{
    size_t n;
    size_t i;
 public:

    TestClass() : n(0) { i = ++_index; };

    void    incrementNum() { ++n; }

    size_t const &getNum() const { return n; };
    size_t const &getIndex() const { return i; };
};

/* Derived to define the actual jobs */

template<typename T>
class ThreadpoolDerived : public Threadpool<T>
{

 public:

    ThreadpoolDerived(size_t w, size_t q) : Threadpool<T>(w, q) {};

    void job(TestClass *task)
    {
        size_t n;
        size_t i;

        task->incrementNum();
        i = task->getIndex();
        n = task->getNum();
        if ((i == 1 || i == WORKERS) && n % PRINT_FREQUENCY == 0)
        {
            pthread_mutex_lock(&writeLock);
            std::cout << "TestClass " << task->getIndex() << ": ";
            std::cout << "completed " << n << " tasks" << std::endl;
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
    ThreadpoolDerived<TestClass> q(WORKERS, QUEUE);
    size_t i;

    pthread_mutex_init(&writeLock, NULL);

    std::cout << "Starting. Will print every " << PRINT_FREQUENCY << "th task.\n" << std::endl;

    q.start();
    for (i = 1; i <= TOTAL_TASKS; ++i)
    {
        if (q.addTask(&tab[ i % WORKERS ]) == false)
            --i;
    }

    while (getResult(tab, WORKERS) < TOTAL_TASKS)
        usleep(10000);

    std::cout << getResult(tab, WORKERS) << " tasks completed" << std::endl;
    pthread_mutex_destroy(&writeLock);
}
