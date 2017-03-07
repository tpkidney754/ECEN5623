#ifndef MUTEXES_H
#define MUTEXES_H

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS     2

typedef struct
{
   double x;
   double y;
   double z;
   double acceleration;
   double roll;
   double pitch;
   double yaw;
   struct timespec sampleTime;
   unsigned int valid;
} Attitude_t;

Attitude_t attitude;
pthread_mutex_t attitudeMut = PTHREAD_MUTEX_INITIALIZER;

char* log_time( char* in){
   struct timespec current;
   struct tm *currentT;
   char str[30];

   clock_gettime( CLOCK_REALTIME, &current );
   currentT = localtime( &current );

   sprintf(str,"%02d:%02d:%02d.%lu ",
                     currentT->tm_hour, currentT->tm_min, currentT->tm_sec,current.tv_nsec);
   printf("%s",str);
   return in;
}

#endif
