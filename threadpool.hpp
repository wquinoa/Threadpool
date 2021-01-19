#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP
# include <pthread.h>
# include <unistd.h>
# include <iostream>
# include <vector>
# include <time.h>
# include <queue>


/* T is the class that performs jobs */

template <typename T>
class Threadpool
{
 
 /* Arbitrary values, don't set them too high. */

 # define MAX_WORK 32
 # define MAX_Q 8192
 # define VALIDATE_CARGS(x, y) (x > 0 || y > 0 || x <= MAX_WORK || y <= MAX_Q)
 
    std::vector<pthread_t >     threads;
    pthread_mutex_t             innerLock;
    std::queue<T *>             q;
    size_t                      qLen;
    
    int flags;

    static void    *workerLoop(void* arg)
    {
        Threadpool<T> *pool = (Threadpool<T> *)arg;
        T *task;
        
        /* Waiting for a task in queue or a signal */
        while (true)
        {

            if ((task = pool->getTask()))
                pool->job(task);

            if (pool->status())
                break ;
            
            usleep(200);
        }
        return (nullptr);
    }

    enum e_pool_flag
    {
        pool_ok,
        pool_stop,
        pool_kill
    };

 public:

    Threadpool(int workers, int queue_size)
    : threads(workers), qLen(queue_size), flags(pool_ok)
    {
        if (!VALIDATE_CARGS(workers, queue_size))
            throw (std::runtime_error("Invalid constructor arguments"));

        if (pthread_mutex_init(&innerLock, nullptr) < 0)
            throw (std::runtime_error("Mutex init failed"));

        pthread_mutex_lock(&innerLock);
        for (size_t i = 0; i < threads.size(); ++i)
        {
            if (pthread_create(&threads[i], nullptr, &workerLoop, this) < 0)
                throw (std::runtime_error("Thread creation failed"));
        }
        pthread_mutex_unlock(&innerLock);
    }

    virtual ~Threadpool()
    {
        flags = pool_stop;

        for (size_t i = 0; i < threads.size(); ++i)
        {
            if (pthread_join(threads[i], nullptr) < 0)
                throw (std::runtime_error("Pthread join failed"));

        }
        pthread_mutex_destroy(&innerLock);
    }

    void addTask(T *value)
    {
        pthread_mutex_lock(&innerLock);
        q.push(value);
        pthread_mutex_unlock(&innerLock);
    }

    T *getTask() 
    {
        T *ret;

        pthread_mutex_lock(&innerLock);
        if (q.empty())
            ret = nullptr;
        else
        {
            ret = q.front();
            q.pop();
        }
        pthread_mutex_unlock(&innerLock);
        return ret;
    }

    int const &status() const { return flags; }

    void kill() { flags = pool_kill; }

    /* Derive from Threadpool to create the needed job */

    virtual void job(T *t) = 0;
    //virtual void job(void) = 0;
    //virtual void *job(T *t, int flag) = 0;
    
};

#endif