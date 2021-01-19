#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP
# include <pthread.h>
# include <unistd.h>
# include <iostream>
# include <vector>
# include <queue>


/* T is the class that performs jobs */

template <typename T>
class Threadpool
{
 
 /* Arbitrary values, don't set them too high. */

 # define MAX_WORK 32
 # define MAX_Q 8192
 # define VALID_PARAMS(x, y) (x > 0 && y > 0 && x <= MAX_WORK && y <= MAX_Q)
 
    std::vector<pthread_t>      threads;
    pthread_mutex_t             innerLock;
    std::queue<T *>             q;
    size_t                      nWorkers, qSize;
    
    int status;

    enum e_pool_status
    {
        pool_ok = 0,
        pool_pause = (1 << 0),
        pool_stop = (1 << 1),
        pool_kill = (1 << 2)
    };

    class pool_strerror : public std::exception {
      public:
        char const *what() const throw() {
            std::string error(strerror(errno)); 

            error += " errno: " + std::to_string(errno);
            return (error.c_str());
        }
    };

    static void    *workerLoop(void* arg)
    {
        Threadpool<T> *pool = (Threadpool<T> *)arg;
        T *task;
        
        while (true)
        {

            while (pool->state() == pool_pause)
                usleep (20000);

            usleep(200);

            /* Waiting for a task in queue or a signal */

            if ((task = pool->getTask()))
                pool->job(task);

            if (pool->state())
                break ;
            
        }
        usleep(20000);
        return (nullptr);
    }

    int const &state() const { return status; }

    /* Cannot copy or assign a Threadpool */

    Threadpool &operator=(const Threadpool &copy) { 
        (void)copy; return (*this); 
    };

    Threadpool(const Threadpool &copy) {
        (void)copy;
    };

    /* pthread wrappers */

    void lock() {
        if (pthread_mutex_lock(&innerLock))
            throw pool_strerror();
    }

    void unlock() {
        if (pthread_mutex_unlock(&innerLock))
            throw pool_strerror();
    }

    void createThreads() {
        pthread_t newThread;

        status = pool_pause;
        for (size_t i = 0; i < nWorkers; ++i)
        {
            if (pthread_create(&newThread, nullptr, &workerLoop, this) < 0)
                throw pool_strerror();
            threads.push_back(newThread);
        }
    }

    void gracefulShutdown() {
        status = pool_stop;

        for (size_t i = 0; i < nWorkers; ++i)
        {
            if (pthread_join(threads[i], nullptr) < 0)
                throw pool_strerror();
        }
    }

 public:

    /* Constructor */

    Threadpool(int workers, int queue_size)
    : nWorkers(workers), qSize(queue_size), status(pool_ok)
    {

        if (VALID_PARAMS(workers, queue_size) == false)
            throw (std::runtime_error("Threadpool: invalid constructor arguments"));

        if (pthread_mutex_init(&innerLock, nullptr) < 0)
            throw pool_strerror();

        createThreads();
    }

    /* Destructor */

    virtual ~Threadpool()
    {
        gracefulShutdown();

        if (pthread_mutex_destroy(&innerLock))
            throw pool_strerror();
    }

    /* Member functions */

    bool addTask(T *value)
    {
        bool ret;

        lock();
        if (q.size() == qSize)
            ret = false;
        else
        {
            q.push(value);
            ret = true;
        }
        unlock();
        return ret;
    }

    T *getTask() 
    {
        T *ret;

        lock();
        if (q.empty() || status != pool_ok)
            ret = nullptr;
        else 
        {
            ret = q.front();
            q.pop();
        }
        unlock();
        return ret;
    }

    /* "Signals" */

    void kill() { status = pool_kill; }

    void pause() { status = pool_pause; }

    void start() { status = pool_ok; }

    /* Derive from Threadpool to create the needed job */

    virtual void job(T *t) = 0;
    //virtual void job(void) = 0;
    //virtual void *job(T *t, int flag) = 0;
    
};

#endif