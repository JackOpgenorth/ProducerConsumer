#include <iostream>
#include <stdio.h>
#include <queue>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <chrono>
#include <sys/resource.h>
#include <iomanip>

#include "main.h"


using namespace std;



void out_log(string msg, int ID){

    pthread_mutex_lock(&logMutex);
    ofstream logWriter;

    logWriter.open(logname, ios_base::app);

    logWriter << setprecision(3) << get_time();
    logWriter << "   ID= " + to_string(ID) + " " + msg << endl;

    logWriter.close();

    pthread_mutex_unlock(&logMutex);
}