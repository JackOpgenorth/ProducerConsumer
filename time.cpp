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


double get_time(){

    auto current  = chrono::high_resolution_clock::now();

    
    chrono::duration<double> t = current - start;

    return (double)t.count();
}