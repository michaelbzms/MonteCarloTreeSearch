#include <iostream>
#include "../include/JobScheduler.h"

#define CHECK_PERROR(call, msg, actions) { if ( (call) < 0 ) { perror(msg); actions } }

using namespace std;


/* JobScheduler Implementation */
JobScheduler::JobScheduler(unsigned int _number_of_threads) : jobs_running(0), number_of_threads(_number_of_threads), t_args(NULL) {
    threads_must_exit = false;
    CHECK_PERROR(pthread_mutex_init(&queue_lock, NULL), "pthread_mutex_t_init failed",)
    CHECK_PERROR(pthread_cond_init(&queue_cond, NULL), "pthread_cond_init failed",)
    CHECK_PERROR(pthread_cond_init(&jobs_finished_cond, NULL), "pthread_cond_init failed",)
    t_args = new struct thread_args(&job_queue, &queue_lock, &queue_cond, &jobs_running, &threads_must_exit, &jobs_finished_cond, &tagged_jobs_pending);
    threads = new pthread_t[number_of_threads];
    for (int i = 0; i < number_of_threads; i++) {
        CHECK_PERROR(pthread_create(&threads[i], NULL, thread_code, (void *) t_args), "pthread_create failed", threads[i] = 0;)
    }
}

JobScheduler::~JobScheduler() {
    waitUntilJobsHaveFinished();     // (!) important
    delete t_args;
    threads_must_exit = true;
    CHECK_PERROR(pthread_cond_broadcast(&queue_cond), "pthread_broadcast failed", )
    for (int i = 0; i < number_of_threads; i++) {
        CHECK_PERROR(pthread_join(threads[i], NULL), "pthread_join failed", )
    }
    delete[] threads;
    CHECK_PERROR(pthread_mutex_destroy(&queue_lock), "pthread_mutex_destroy failed", )
    CHECK_PERROR(pthread_cond_destroy(&queue_cond), "pthread_cond_destroy failed", )
    CHECK_PERROR(pthread_cond_destroy(&jobs_finished_cond), "pthread_cond_destroy failed", )
}

void JobScheduler::schedule(Job *job) {
    CHECK_PERROR(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock failed", )
    if (job->TAG != NOTAG){
        auto it = tagged_jobs_pending.find(job->TAG);
        if (it == tagged_jobs_pending.end()) tagged_jobs_pending.insert(make_pair(job->TAG, 1));
        else (it->second)++;
    }
    job_queue.push(job);
    CHECK_PERROR(pthread_cond_signal(&queue_cond), "pthread_cond_signal failed", )
    CHECK_PERROR(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock failed", )
}

bool JobScheduler::JobsHaveFinished(int tag) {
    bool result;
    if (tag == NOTAG){
        CHECK_PERROR(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock failed", )
        result = job_queue.empty() && jobs_running == 0;
        CHECK_PERROR(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock failed", )
    } else {
        CHECK_PERROR(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock failed", )
        auto it = tagged_jobs_pending.find(tag);
        if (it != tagged_jobs_pending.end()){
            result = it->second == 0;
        } else {
            result = true;
        }
        CHECK_PERROR(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock failed", )
    }
    return result;
}

void JobScheduler::waitUntilJobsHaveFinished(int tag) {
    if (tag == NOTAG){
        CHECK_PERROR(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock failed", )
        while (!job_queue.empty() || jobs_running > 0){
            CHECK_PERROR(pthread_cond_wait(&jobs_finished_cond, &queue_lock) , "pthread_cond_wait failed", )
        }
        CHECK_PERROR(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock failed", )
    } else {
        CHECK_PERROR(pthread_mutex_lock(&queue_lock), "pthread_mutex_lock failed", )
        auto it = tagged_jobs_pending.find(tag);
        if (it != tagged_jobs_pending.end()){
            while (it->second > 0){
                CHECK_PERROR(pthread_cond_wait(&jobs_finished_cond, &queue_lock) , "pthread_cond_wait failed", )
            }
        }
        CHECK_PERROR(pthread_mutex_unlock(&queue_lock), "pthread_mutex_unlock failed", )
    }
}


/* Thread logic */
void *thread_code(void *args) {
    struct thread_args *argptr = ((struct thread_args *) args);
    queue<Job *> *job_queue = argptr->jobQueue;
    pthread_mutex_t *queue_lock = argptr->queueLock;
    pthread_cond_t *queue_cond = argptr->queueCond;
    volatile unsigned int *jobs_running_ptr = argptr->jobsRunning;
    volatile bool *threads_must_exit_ptr = argptr->threads_must_exit;
    pthread_cond_t *jobs_finished_cond_ptr = argptr->jobs_finished_cond;
    unordered_map<int, volatile unsigned int> *tagged_jobs_pending_ptr = argptr->tagged_jobs_pending;

    while (!(*threads_must_exit_ptr)) {
        Job *job;
        int tag;

        CHECK_PERROR(pthread_mutex_lock(queue_lock), "pthread_mutex_lock failed", continue; )
        while (job_queue->empty() && !(*threads_must_exit_ptr)){
            CHECK_PERROR(pthread_cond_wait(queue_cond, queue_lock) , "pthread_cond_wait failed", )
        }
        if (*threads_must_exit_ptr){
            CHECK_PERROR(pthread_mutex_unlock(queue_lock), "pthread_mutex_unlock failed", )
            break;
        }
        job = job_queue->front();       // get pointer to next job
        job_queue->pop();               // remove it from queue
        (*jobs_running_ptr)++;
        CHECK_PERROR(pthread_mutex_unlock(queue_lock), "pthread_mutex_unlock failed", )

        tag = job->TAG;
        job->run();        
        delete job;                     // (!) scheduler deletes scheduled objects when done but they must be allocated before they get scheduled

        CHECK_PERROR(pthread_mutex_lock(queue_lock), "pthread_mutex_lock failed", )
        int num = 0;
        if (job->TAG != NOTAG){
            num = --(*tagged_jobs_pending_ptr)[job->TAG];
        }
        (*jobs_running_ptr)--;
        if ( num == 0 || (*jobs_running_ptr) == 0 ){
            CHECK_PERROR(pthread_cond_broadcast(jobs_finished_cond_ptr), "pthread_cond_signal failed", )
        }
        CHECK_PERROR(pthread_mutex_unlock(queue_lock), "pthread_mutex_unlock failed", )
    }
    pthread_exit((void *) 0);
}
