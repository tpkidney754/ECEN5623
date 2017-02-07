//------------------------------------------------------------------------------
// ECEN 5623 - Spring 2017
//   Exercise 1
//   Tyler Kidney & Robert Blazewicz
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <syslog.h>
#include <stdint.h>
#include <sys/sysinfo.h>
#include <assert.h>

#define CeMaxTasks      ( 3 )
#define NUM_CPUS        ( 1 )
#define CeXXXX_Quantum  ( 3 )
#define SCHED_POLICY    ( SCHED_FIFO )
#define CbTRUE          ( 1 )
#define CbFALSE         ( 0 )
#define NSEC_PER_SEC    ( 1000000000 )
#define CeERROR         ( -1 )
#define CeOK            ( 0 )

//------------------------------------------------------------------------------

extern void *ProcXXXX_Task1( void *threadp );
//extern void *ProcXXXX_Task2( void *threadp );
//extern void *ProcXXXX_Task3( void *threadp );

//------------------------------------------------------------------------------

typedef uint8_t TbBOOLEAN;
typedef uint8_t BYTE;

typedef void *( *TeXXXX_Ptr_Tasks )( void *threadp );

typedef struct TeXXXX_h_ThreadParams
{
   uint32_t e_Idx_Thread;
   int32_t e_Cnt_T;
   int32_t e_Cnt_C;
} TeXXXX_h_ThreadParams;

typedef void *( *TeXXX_ptr_Tasks )( void *threadp );

//------------------------------------------------------------------------------

// Vector table of functions to invoke for each thread created.
const TeXXXX_Ptr_Tasks VaXXXX_Ptr_Tasks[ CeMaxTasks ] =
{
   ProcXXXX_Task1,
   ProcXXXX_Task1,
   ProcXXXX_Task1
};

// Table of constants for each thread to define schedule operation.
const TeXXXX_h_ThreadParams VaXXXX_h_ThreadParams[CeMaxTasks] =
{
   { 0, 2, 1 },
   { 1, 5, 2 },
   { 2, 10, 1 }
};

//------------------------------------------------------------------------------

// POSIX thread declarations and scheduling attributes
pthread_attr_t rt_sched_attr[ CeMaxTasks ];
pthread_t VaXXXX_h_PThreads[ CeMaxTasks ];
struct sched_param rt_param[ CeMaxTasks ];
struct timespec taskStartTime[ CeMaxTasks ];
struct timespec task_stop_time = { 0, 0 };
int32_t numberOfProcessors;
int32_t rt_max_prio;
int32_t rt_min_prio;
struct sched_param tstThread_param;
pthread_attr_t tstThread_sched_attr;
struct timespec rtclk_resolution;

//------------------------------------------------------------------------------
// print_scheduler
//   Print the current schedule policy.

void print_scheduler( void )
{
   uint32_t schedType;

   schedType = sched_getscheduler( getpid( ) );

   switch( schedType )
   {
      case SCHED_FIFO:
         printf( "Pthread Policy is SCHED_FIFO\n" );
         break;
      case SCHED_OTHER:
         printf( "Pthread Policy is SCHED_OTHER\n" );
         break;
      case SCHED_RR:
         printf( "Pthread Policy is SCHED_OTHER\n" );
         break;
      default:
         printf( "Pthread Policy is UNKNOWN\n" );
   }
}

//------------------------------------------------------------------------------
// delta_t
//   Compute the delta between two timespec structures. This function assumes
//   time values are not monotonic and their may be inversion due to the use
//   of CLOCK_REALTIME rather than the slower and stable CLOCK_MONOTONIC.

int32_t delta_t( struct timespec *stop, struct timespec *start, struct timespec *delta_t )
{
   int32_t dt_sec=stop->tv_sec - start->tv_sec;
   int32_t dt_nsec=stop->tv_nsec - start->tv_nsec;

   if( dt_sec >= 0 )
   {
      if( dt_nsec >= 0 )
      {
         delta_t->tv_sec = dt_sec;
         delta_t->tv_nsec = dt_nsec;
      }
      else
      {
         delta_t->tv_sec = dt_sec - 1;
         delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
      }
   }
   else
   {
      if( dt_nsec >= 0 )
      {
         delta_t->tv_sec = dt_sec;
         delta_t->tv_nsec = dt_nsec;
      }
      else
      {
         delta_t->tv_sec = dt_sec - 1;
         delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
      }
   }

   return CeOK;
}

//------------------------------------------------------------------------------
// ProcXXXX_Task1
//   Task code used to
//     1. busy loop for C
//     2. sleep until T
//     3. repeat until shutdown time reached

void *ProcXXXX_Task1(void *threadp)
{
   int32_t LeXXXX_Cnt_Index;
   int32_t interval;
   pthread_t thread;
   int32_t cpucore;
   cpu_set_t cpuset;
   TeXXXX_h_ThreadParams *VaXXXX_h_ThreadParams = (TeXXXX_h_ThreadParams *)threadp;
   struct timespec rtclk_start_time = { 0, 0 };
   struct timespec rtclk_stop_time = { 0, 0 };
   struct timespec rtclk_dt = { 0, 0 };
   struct timespec rtclk_spin_dt = { 0, 0 };
   struct timespec interval_stop_time = { 0, 0 };
   struct timespec sleep_time = { 0, 0 };
   struct timespec remaining_time = { 0, 0 };

   // Counter used to track invocations of each thread.
   interval = 0;

   // Compute the T and C interval in seconds.
   int32_t T = CeXXXX_Quantum * VaXXXX_h_ThreadParams->e_Cnt_T;
   int32_t C = CeXXXX_Quantum * VaXXXX_h_ThreadParams->e_Cnt_C;

   // Set the core affinity
   thread = pthread_self( );
   cpucore = sched_getcpu( );
   CPU_ZERO( &cpuset );
   pthread_getaffinity_np( thread, sizeof( cpu_set_t ), &cpuset );

   { // Display CPU and core status
     printf( "Thread %d on core %d, affinity contained:", VaXXXX_h_ThreadParams->e_Idx_Thread, cpucore );
     for( LeXXXX_Cnt_Index = 0; LeXXXX_Cnt_Index < numberOfProcessors; LeXXXX_Cnt_Index++ )
     {
       if( CPU_ISSET( LeXXXX_Cnt_Index, &cpuset ) )
       {
         printf(" CPU-%d ", LeXXXX_Cnt_Index);
       }
     }
     printf( "\n" );
   }

   // Look until the schedule stop time is reached
   do
   {
      interval++;

      interval_stop_time.tv_sec = taskStartTime[ VaXXXX_h_ThreadParams->e_Idx_Thread ].tv_sec + T;
      interval_stop_time.tv_nsec = taskStartTime[ VaXXXX_h_ThreadParams->e_Idx_Thread ].tv_nsec;

      delta_t( &interval_stop_time, &taskStartTime[ VaXXXX_h_ThreadParams->e_Idx_Thread ], &rtclk_dt );

      syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
              "Thread %d - %d: T = %d C = %d",
              VaXXXX_h_ThreadParams->e_Idx_Thread, interval, T, C );

      clock_gettime( CLOCK_REALTIME, &rtclk_start_time );
      uint32_t pre_sec = 0;
      do
      {
         clock_gettime( CLOCK_REALTIME, &rtclk_stop_time );
         delta_t( &rtclk_stop_time, &rtclk_start_time, &rtclk_spin_dt );
         if( rtclk_spin_dt.tv_sec > pre_sec )
         {
            if( rtclk_spin_dt.tv_sec > pre_sec + 1 )
            {
               clock_gettime(CLOCK_REALTIME, &rtclk_start_time);
               clock_gettime(CLOCK_REALTIME, &rtclk_stop_time);
               delta_t(&rtclk_stop_time, &rtclk_start_time, &rtclk_spin_dt);
            }
            else
            {
               delta_t( &interval_stop_time, &rtclk_stop_time, &rtclk_dt );
               syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
                       "Thread %d - %d: Spin",
                       VaXXXX_h_ThreadParams->e_Idx_Thread, interval );
            }
         }

         pre_sec = rtclk_spin_dt.tv_sec;
         delta_t( &interval_stop_time, &rtclk_stop_time, &rtclk_dt );
      }
      while( rtclk_spin_dt.tv_sec < C && rtclk_dt.tv_sec > 0 );

      delta_t( &interval_stop_time, &rtclk_stop_time, &rtclk_dt );
      sleep_time.tv_sec = rtclk_dt.tv_sec;
      sleep_time.tv_nsec = rtclk_dt.tv_nsec;

      if( sleep_time.tv_sec >= 0 && sleep_time.tv_nsec >= 0 )
      {
         syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
                 "Thread %d - %d: Sleep %ld.%09lds",
                 VaXXXX_h_ThreadParams->e_Idx_Thread, interval,
                 sleep_time.tv_sec, sleep_time.tv_nsec );

         nanosleep( &sleep_time, &remaining_time );
      }

      delta_t( &task_stop_time, &rtclk_stop_time, &rtclk_dt );

      clock_gettime( CLOCK_REALTIME, &taskStartTime[ VaXXXX_h_ThreadParams->e_Idx_Thread ] );
   }
   while( rtclk_dt.tv_sec > 0 );

   syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
           "Thread %d - %d: Stop",
           VaXXXX_h_ThreadParams->e_Idx_Thread, interval);

   pthread_exit( threadp );
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   int32_t LeXXXX_Cnt_Index;
   int32_t LeXXXX_i_Result;
   int32_t idx, scope;
   pthread_attr_t main_sched_attr;
   struct sched_param main_param;
   cpu_set_t allcpuset;
   cpu_set_t threadcpu;
   int32_t coreid;
   struct timespec task_start_time = {0, 0};

   // Open syslog using the USER facility
   openlog( NULL, LOG_CONS, LOG_USER );

   printf( "This system has %d processors configured and %d processors available.\n",
           get_nprocs_conf( ), get_nprocs( ) );

   numberOfProcessors = get_nprocs_conf( );
   printf( "number of CPU cores=%d\n", numberOfProcessors );

   CPU_ZERO( &allcpuset );

   for( LeXXXX_Cnt_Index = 0; LeXXXX_Cnt_Index < numberOfProcessors; LeXXXX_Cnt_Index++ )
   {
      CPU_SET( LeXXXX_Cnt_Index, &allcpuset );
   }

   if( numberOfProcessors >= NUM_CPUS )
   {
      printf( "Using sysconf number of CPUS=%d, count in set=%d\n",
              numberOfProcessors, CPU_COUNT( &allcpuset ) );
   }
   else
   {
      numberOfProcessors = NUM_CPUS;
      printf( "Using DEFAULT number of CPUS=%d\n", numberOfProcessors );
   }

   print_scheduler( );
   pthread_attr_init( &main_sched_attr );
   pthread_attr_setinheritsched( &main_sched_attr, PTHREAD_EXPLICIT_SCHED );
   pthread_attr_setschedpolicy( &main_sched_attr, SCHED_FIFO );

   rt_max_prio = sched_get_priority_max( SCHED_FIFO );
   rt_min_prio = sched_get_priority_min( SCHED_FIFO );

   main_param.sched_priority = rt_max_prio;
   LeXXXX_i_Result = sched_setscheduler( getpid( ), SCHED_FIFO, &main_param );

   if( LeXXXX_i_Result < 0 )
   {
      printf( "ERROR; sched_setscheduler is %d\n", LeXXXX_i_Result );
      perror( "sched_setschduler" );
      exit( -1 );
   }

   print_scheduler( );

   pthread_attr_getscope( &main_sched_attr, &scope );

   if( scope == PTHREAD_SCOPE_SYSTEM )
   {
      printf( "PTHREAD SCOPE SYSTEM\n" );
   }
   else if( scope == PTHREAD_SCOPE_PROCESS )
   {
      printf( "PTHREAD SCOPE PROCESS\n" );
   }
   else
   {
      printf( "PTHREAD SCOPE UNKNOWN\n" );
   }

   printf( "rt_max_prio=%d\n", rt_max_prio );
   printf( "rt_min_prio=%d\n", rt_min_prio );

   if ( clock_getres( CLOCK_REALTIME, &rtclk_resolution ) == CeERROR )
   {
      perror( "clock_getres" );
      exit( -1 );
   }
   else
   {
      printf( "\n\nPOSIX RT clock resolution:\n\t%ld secs, %ld microsecs, %ld nanosecs\n",
              rtclk_resolution.tv_sec,
              ( rtclk_resolution.tv_nsec / 1000 ),
              rtclk_resolution.tv_nsec );
   }

   // Get the start time for the schedule
   clock_gettime( CLOCK_REALTIME, &task_start_time );

   // Set the schedule stop time used by the tasks to shutdown.
   task_stop_time.tv_sec = task_start_time.tv_sec + ( CeXXXX_Quantum * 10 * 2 );
   task_stop_time.tv_nsec = task_start_time.tv_nsec;

   // Create the tasks
   for( LeXXXX_Cnt_Index = 0; LeXXXX_Cnt_Index < CeMaxTasks; LeXXXX_Cnt_Index++ )
   {
      assert( LeXXXX_Cnt_Index == VaXXXX_h_ThreadParams[LeXXXX_Cnt_Index].e_Idx_Thread );

      // Set the start time used by each thread to the same initial value.
      taskStartTime[ LeXXXX_Cnt_Index ].tv_sec = task_start_time.tv_sec;
      taskStartTime[ LeXXXX_Cnt_Index ].tv_nsec = task_start_time.tv_nsec;

      CPU_ZERO( &threadcpu );
      coreid = 0;
      { // Display CPU and core status
        printf( "Setting thread %d to core %d:", LeXXXX_Cnt_Index, coreid );
        CPU_SET( coreid, &threadcpu );
        for( idx=0; idx<numberOfProcessors; idx++ )
        {
          if(CPU_ISSET( idx, &threadcpu ) )
          {
            printf( " CPU-%d", idx );
          }
        }
        printf( "\n" );
      }

      LeXXXX_i_Result = pthread_attr_init( &rt_sched_attr[ LeXXXX_Cnt_Index ] );
      LeXXXX_i_Result = pthread_attr_setinheritsched( &rt_sched_attr[ LeXXXX_Cnt_Index ], PTHREAD_EXPLICIT_SCHED );
      LeXXXX_i_Result = pthread_attr_setschedpolicy( &rt_sched_attr[ LeXXXX_Cnt_Index ], SCHED_FIFO );
      LeXXXX_i_Result = pthread_attr_setaffinity_np( &rt_sched_attr[ LeXXXX_Cnt_Index ], sizeof( cpu_set_t ), &threadcpu );

      rt_param[ LeXXXX_Cnt_Index ].sched_priority = rt_max_prio - LeXXXX_Cnt_Index - 1;
      pthread_attr_setschedparam( &rt_sched_attr[ LeXXXX_Cnt_Index ], &rt_param[ LeXXXX_Cnt_Index ] );

      pthread_create( &VaXXXX_h_PThreads[ LeXXXX_Cnt_Index ],
                      ( void *)&rt_sched_attr[LeXXXX_Cnt_Index ],
                      VaXXXX_Ptr_Tasks[LeXXXX_Cnt_Index ],
                      ( void *)&( VaXXXX_h_ThreadParams[ LeXXXX_Cnt_Index ] ) );

      // Delay the start of each task by 1 second so that they don't start out of order.
      sleep( 1 );
   }

   // Wait for the tasks to terminate
   for( LeXXXX_Cnt_Index = 0; LeXXXX_Cnt_Index < CeMaxTasks; LeXXXX_Cnt_Index++ )
   {
      pthread_join( VaXXXX_h_PThreads[ LeXXXX_Cnt_Index ], NULL );
   }

   // Close syslog
   closelog( );

   printf( "TEST COMPLETE\n" );
}
