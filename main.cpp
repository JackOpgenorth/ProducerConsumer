#include <iostream>
#include <stdio.h>
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <chrono>
#include <iomanip>

#include "main.h"
#include "tands.cpp"
using namespace std;



queue<order> orders;

pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t fullCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t emptyCond = PTHREAD_COND_INITIALIZER;


bool full = 0; // C++ queues don't have a size, I will use this to pretend it does
bool empty = 1;

bool finished = 0;

int Q;
int length;// for the queue

// Defining variables for the summary at the end
int asks = 0;
int receives = 0;
int completes = 0;
int works = 0;
int sleeps = 0;

string logname = "prodcon.log";



// clock.h was not working correctly for some reason, so I'm using chrono
chrono::time_point<chrono::high_resolution_clock> start;


void * consume_routine(void * args){


    // Note: Thread num is NOT the thread ID.
    int *thread_num = (int*)args;

    int* tasks; // The number of tasks completed by this thread
    tasks = new int;


    int tasks_num = 0;

    pthread_t t_id = pthread_self();


    string msg; // Will go into the log
    
    bool alreadyAsked = 0; // Threads only ask once before they get work
    
    
    while (!finished || !orders.empty()){


        if (!alreadyAsked){
            out_log("        Ask", *thread_num + 1);
            asks++;
            alreadyAsked = 1;
        }
        order toBeProcessed;

        pthread_mutex_lock(&queueMutex);//  lock now, since we are accessing the queue
        empty = orders.empty(); 

        if (empty){


            if (finished){
                //If we are empty and the producer is finished, then we don't want to wait for anything
                pthread_mutex_unlock(&queueMutex);
                break;
            }


            // In order to wait when the queue it empty, I chose to use condtion variables. The consumer will be woken up by the producer
            pthread_cond_wait(&emptyCond, &queueMutex);
            pthread_mutex_unlock(&queueMutex);
            
            // At this point there is more work in the queue, so we return to the start and try again
            continue;
        }


            
  
        receives++;
        //Take from queue
        toBeProcessed = orders.front();
        orders.pop();

        int n = toBeProcessed.n;
        Q--;
        msg = "Q= " + to_string(Q) + "    Receive     " + to_string(n);
        out_log(msg, *thread_num + 1);// Writing the Receive to the log

        if (full){//  wake up the producer if needed
            pthread_cond_signal(&fullCond);
            full = 0;
        }

        // Finally won't be touching any shared variables, so we can unlock
        pthread_mutex_unlock(&queueMutex);

        Trans(n);
        tasks_num++;
        completes++;
        
        msg = "        Complete    " + to_string(n);

        out_log(msg, *thread_num + 1);
        alreadyAsked = 0;// Now that we have completed an order, we can ask again


    }

    //last but not least, return the number of tasks the thread compleated

    *tasks = tasks_num;

    
    return tasks;


}


void producer_routine(){

    char inBuf[100]; // This number is arbitrary, just want to be absolutly sure we do not index out of bounds

    while (!finished){
        order newOrder;

        if (!(cin >> inBuf)){ // read until EOF
            finished = 1;

            break;
        }
        

        // parse input
        char parsed[100];
        int i = 1;
        while (inBuf[i] != '\0'){
            parsed[i - 1] = inBuf[i];
            i++;
        }
        parsed[i - 1] = '\0';

        int n = atoi(parsed); // The n as specified in the assignment description

        if (inBuf[0] == 'S'){

            string msg = "        Sleep       " + to_string(n);
            out_log(msg, 0);
            Sleep(n);
            sleeps++;
            continue;
        }
        else{
            works++;
            
            newOrder.n =  n;


        }
            

        // lock the queue and add to it
        pthread_mutex_lock(&queueMutex);

        if (orders.size() >= length){
            full = 1;
        }

        if(full){
            // use condition variables to block the producer. Will eventualy be woken up when a thread takes work
            pthread_cond_wait(&fullCond,&queueMutex); 

        }

        Q++;
                    
        string msg = "Q= " + to_string(Q) + "    Work        " + to_string(n);
        out_log(msg, 0); // 0 is the producers ID
        


        orders.push(newOrder);

        // Need to wake up all threads waiting for work
        if (empty){
            pthread_cond_broadcast(&emptyCond);
        }


        pthread_mutex_unlock(&queueMutex);
    }


}


int main(int argc, char** argv){
    start = chrono::high_resolution_clock::now(); // measure all time relative to this


    int numthreads = atoi(argv[1]);
    length = numthreads * 2;

    int argNum = argc;
    
    if (argNum == 3){
        string num = argv[2];
        logname = "prodcon." + num + ".log";
    }

    remove(logname.c_str()); // delete any log if it exists

    pthread_t threadId;

    pthread_t threadList[numthreads]; // Will hold all thread IDs

    int threadNums[numthreads];

    int i;
    for (int i = 0; i < numthreads; i++){
        
        threadNums[i] = i;

        if (pthread_create(&threadId, NULL, consume_routine, (void*) &threadNums[i])){
            printf("error\n");
        }
        threadList[i] = threadId;
    }

    producer_routine();
    
    // Need to wait for all threads
    int taskNumList[numthreads];
    for (int i = 0; i < numthreads; i++){

        pthread_cond_broadcast(&emptyCond); // catch any threads who might be waiting because of any empty queue

        void* return_val;
        pthread_join(threadList[i], &return_val);

        taskNumList[i] = *(int*)return_val;
        delete (int*)return_val;

    }
    double t = get_time();  

    double transPerSec = (float)works/t;

    //Write the summary
    
    string workDone = "Work:       "  + to_string(works) + "\n";
    string askDone = "Ask:        "  + to_string(asks) + "\n";
    string receiveDone = "Receive:    "  + to_string(receives) + "\n";
    string completeDone = "Complete:   "  + to_string(completes) + "\n";
    string sleepDone = "Sleep:      "  + to_string(sleeps) + "\n";

    ofstream output;
    output.open(logname, ios_base::app);
    output << "Summary:\n";
    output << workDone + askDone + receiveDone + completeDone + sleepDone;

    
    for (int i = 0; i < numthreads; i++){

        
        output << "Thread " + to_string(i) + "    " + to_string(taskNumList[i]) + '\n';
        
        
    }



    string msg = "Transactions per second: " + to_string(transPerSec) + "\n";

    output << msg;

    output.close();

}