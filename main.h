#ifndef MAIN

#define MAIN

#include <string>
#include <queue>

using namespace std;


typedef struct order {
    int threadNum;

    int id;

    int n;


} order;



extern queue<order> orders;// C++ queues don't have a size, so I will pretend it does

extern pthread_mutex_t queueMutex;
extern pthread_cond_t fullCond;
extern pthread_cond_t emptyCond;
extern pthread_mutex_t logMutex;

extern bool full;
extern bool empty;
extern bool finished;

extern int Q;
extern int length;

extern int asks;
extern int receives;
extern int completes;
extern int works;
extern int sleeps;

extern chrono::time_point<chrono::high_resolution_clock> start;
extern string logname;



double get_time();

void out_log(string msg, int ID);




void producer_routine();

#endif