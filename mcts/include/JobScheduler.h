#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <queue>
#include <unordered_map>
#include <pthread.h>
#include <stdlib.h>


#define NUMBER_OF_THREADS 4                  // default number of threads
#define NOTAG -1


using namespace std;


class Job {                                  // extend this to whatever job you want scheduled (in a different .cpp/.h)
public:
    const int TAG;                           // used to group waiting for different jobs
    Job(int tag = NOTAG) : TAG(tag) {}
    virtual ~Job() {}
    virtual void run() = 0;                  // must be thread_safe
};


struct thread_args {
    queue <Job *> *jobQueue;
    pthread_mutex_t *queueLock;
    pthread_cond_t *queueCond;
    volatile unsigned int *jobsRunning;
    volatile bool *threads_must_exit;
    pthread_cond_t *jobs_finished_cond;
    unordered_map<int, volatile unsigned int> *tagged_jobs_pending;
    thread_args(queue<Job *> *_jobQueue, pthread_mutex_t *_queueLock, pthread_cond_t *_queueCond,
                volatile unsigned int *_jobsRunning, volatile bool *_threads_must_exit, pthread_cond_t *_jobs_finished_cond, unordered_map<int, volatile unsigned int> *_tagged_jobs_pending)
            : jobQueue(_jobQueue), queueLock(_queueLock), queueCond(_queueCond), jobsRunning(_jobsRunning), threads_must_exit(_threads_must_exit), jobs_finished_cond(_jobs_finished_cond), tagged_jobs_pending(_tagged_jobs_pending) {}
};

void *thread_code(void *args);


class JobScheduler {
    /* Thread pool */
    pthread_t *threads;
    const unsigned int number_of_threads;
    volatile bool threads_must_exit;
    /* Job queue */
    queue<Job *> job_queue;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;               // true -> not empty
    thread_args *t_args;
    /* Job Info */
    pthread_cond_t jobs_finished_cond;
    volatile unsigned int jobs_running;      // protected by queue_lock
    unordered_map<int, volatile unsigned int> tagged_jobs_pending;   // tag -> number of jobs
public:
    JobScheduler(unsigned int _number_of_threads = NUMBER_OF_THREADS);
    ~JobScheduler();                         // Waits until all jobs have finished!
    void schedule(Job *job);
    bool JobsHaveFinished(int tag = NOTAG);
    void waitUntilJobsHaveFinished(int taf = NOTAG);
    unsigned int get_number_of_threads() const { return number_of_threads; }
};

#endif
