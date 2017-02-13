// Sam Siewert, July 2016
//
// Check to ensure all your CPU cores on in an online state.
//
// Check /sys/devices/system/cpu or do lscpu.
//
// Tegra is normally configured to hot-plug CPU cores, so to make all available,
// as root do:
//
// echo 0 > /sys/devices/system/cpu/cpuquiet/tegra_cpuquiet/enable
// echo 1 > /sys/devices/system/cpu/cpu1/online
// echo 1 > /sys/devices/system/cpu/cpu2/online
// echo 1 > /sys/devices/system/cpu/cpu3/online

#define _GNU_SOURCE

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <stdint.h>
#include <syslog.h>

#define NUM_THREADS       3
#define NUM_CPUS          1
#define NSEC_PER_SEC      1000000000
#define NSEC_PER_MSEC     1000000
#define NSEC_PER_MICROSEC 1000
#define DELAY_TICKS       1
#define ERROR             -1
#define OK                0

uint32_t T[ NUM_THREADS ] = { 2, 5, 10 };
uint32_t C[ NUM_THREADS ] = { 1, 2, 1 };

uint32_t idx = 0, jdx = 1;
uint32_t seqIterations = 47;
uint32_t reqIterations = 10000000;
volatile uint32_t fib = 0, fib0 = 0, fib1 = 1;

int32_t numberOfProcessors;

#define FIB_TEST( seqCnt, iterCnt )        \
   for( idx = 0; idx < ( iterCnt ); idx++ )\
   {                                       \
      fib = fib0 + fib1;                   \
      while( jdx < ( seqCnt ) )            \
      {                                    \
         fib0 = fib1;                      \
         fib1 = fib;                       \
         fib = fib0 + fib1;                \
         jdx++;                            \
      }                                    \
   }                                       \

typedef struct
{
   int32_t threadIdx;
   int32_t T;
   int32_t C;
   struct timespec previousRelease;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//

/*struct sched_param
  {
      int32_t  sched_priority;
      int32_t  sched_curpriority;
      union
      {
         int32_t  reserved[8];
         struct
         {
            int32_t  __ss_low_priority;
            int32_t  __ss_max_repl;
            struct timespec     __ss_repl_period;
            struct timespec     __ss_init_budget;
        } __ss;
      } __ss_un;
}*/
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int32_t rt_max_prio, rt_min_prio;
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;
pthread_attr_t main_attr;
pid_t mainpid;

// returns the time that has passed between start and stop.
int32_t delta_t( struct timespec *stop, struct timespec *start, struct timespec *delta_t )
{
  int32_t dt_sec = stop->tv_sec - start->tv_sec;
  int32_t dt_nsec = stop->tv_nsec - start->tv_nsec;

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

  return( 1 );
}

// Function that each thread is running.
void *counterThread( void *threadp )
{
   int32_t sum = 0, i, cpucore;
   pthread_t thread;
   cpu_set_t cpuset;
   struct timespec start_time = { 0, 0 };
   struct timespec finish_time = { 0, 0 };
   struct timespec thread_dt = { 0, 0 };
   threadParams_t *threadParams = ( threadParams_t *)threadp;

   clock_gettime( CLOCK_REALTIME, &start_time );
   // Gets id of the calling thread
   thread = pthread_self( );
   // Gets the CPU on which the thread is running.
   cpucore = sched_getcpu( );
   // Zero's out the CPU set.
   CPU_ZERO( &cpuset );
   // returns the CPU affinity mask of the thread in the buffer pointed to by cpuset.
   pthread_getaffinity_np( thread, sizeof( cpu_set_t ), &cpuset );

   // COMPUTE SECTION
   uint32_t running = 1;
   i = 0;
   clock_gettime( CLOCK_REALTIME, &start_time );
   while( running )
   {
      sum = sum + i++;
      clock_gettime( CLOCK_REALTIME, &finish_time );
      delta_t( &finish_time, &start_time, &thread_dt );
      if( thread_dt.tv_nsec > threadParams->C )
      {
         running = 0;
      }
   }

      FIB_TEST( seqIterations, reqIterations );
   // END COMPUTE SECTION

/*   printf( "\nThread idx = %d, sum[ 0...%d ] = %d\n",
           threadParams->threadIdx,
           ( ( threadParams->threadIdx) + 1 ) * 100, sum );

   printf( "Thread idx = %d ran on core = %d, affinity contained:",
           threadParams->threadIdx, cpucore );

   for( i = 0; i < numberOfProcessors; i++ )
   {
      if( CPU_ISSET( i, &cpuset ) )
      {
         printf(" CPU-%d ", i);
      }
   }

   printf( "\n" );

   clock_gettime( CLOCK_REALTIME, &finish_time );
   delta_t( &finish_time, &start_time, &thread_dt );

   printf( "\nThread idx = %d ran %ld sec, %ld msec ( %ld microsec )\n",
           threadParams->threadIdx,
           thread_dt.tv_sec,
           ( thread_dt.tv_nsec / NSEC_PER_MSEC ),
           ( thread_dt.tv_nsec / NSEC_PER_MICROSEC ) );
*/
   pthread_exit( &sum );
}


void print_scheduler( )
{
   int32_t schedType;

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

int32_t main( int32_t argc, uint8_t *argv[ ] )
{
   int32_t rc;
   int32_t i, scope, idx;
   cpu_set_t allcpuset;
   cpu_set_t threadcpu;
   int32_t coreid;

   printf( "This system has %d processors configured and %d processors available.\n",
            get_nprocs_conf( ), get_nprocs( ) );
   // Retriving number of processors
   numberOfProcessors = get_nprocs_conf( );
   // Clears out the CPU set.
   CPU_ZERO( &allcpuset );

   for( i = 0; i < numberOfProcessors; i++ )
   {
      // Creates a list of available CPUs
      CPU_SET( i, &allcpuset );
   }

   if( numberOfProcessors >= NUM_CPUS )
   {
      printf( "Using sysconf number of CPUS = %d, count in set = %d\n",
              numberOfProcessors, CPU_COUNT( &allcpuset ) );
   }
   else
   {
      numberOfProcessors = NUM_CPUS;
      printf( "Using DEFAULT number of CPUS = %d\n", numberOfProcessors );
   }
   // Gets the process id of the calling process.
   mainpid = getpid( );
   // Getting the max and min priority of the FIFO Scheduler
   rt_max_prio = sched_get_priority_max( SCHED_FIFO );
   rt_min_prio = sched_get_priority_min( SCHED_FIFO );
   // Prints out the scheduler policy of the main process.
   print_scheduler( );

   // sched_getparam( pid, struct *param )
   // Retrieves the current scheduling parameters for pid and populates param
   rc = sched_getparam( mainpid, &main_param );
   // Setting the main priority to max priority in the struct.
   main_param.sched_priority = rt_max_prio;
   // Sets both the scheduling policy and the parametners for the pid passed in.
   rc = sched_setscheduler( getpid( ), SCHED_FIFO, &main_param );
   // RC is the return value of the called functions. Anything less than zero is an error.
   if( rc < 0 )
   {
      perror( "main_param" );
   }

   print_scheduler( );
   // set/get contention scope atrribute in thread attributes object
   /*POSIX.1 specifies two possible values for scope:

    PTHREAD_SCOPE_SYSTEM
           The thread competes for resources with all other threads in all processes on the system that are in the
           same scheduling allocation domain (a group of one or more processors). PTHREAD_SCOPE_SYSTEM threads are
           scheduled relative to one another according to their scheduling policy and priority.

    PTHREAD_SCOPE_PROCESS
           The thread competes for resources with all other threads in the same process that were also created with
           the PTHREAD_SCOPE_PROCESS contention scope. PTHREAD_SCOPE_PROCESS threads are scheduled relative to other
           threads in the process according to their scheduling policy and priority. POSIX.1 leaves it unspecified
           how these threads contend with other threads in other process on the system or with other threads in the
           same process that were created with the PTHREAD_SCOPE_SYSTEM contention scope.
*/
   pthread_attr_getscope( &main_attr, &scope );

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

   printf( "rt_max_prio = %d\n", rt_max_prio );
   printf( "rt_min_prio = %d\n", rt_min_prio );

   for( i = 0; i < NUM_THREADS; i++ )
   {
      CPU_ZERO( &threadcpu );
      coreid = i % numberOfProcessors;
      printf("Setting thread %d to core %d\n", i, coreid );
      // Setting the coreid to the set
      CPU_SET( coreid, &threadcpu );
      // Looping through all the available processors and checking
      // which CPUs are part of the set.
      for( idx = 0; idx < numberOfProcessors; idx++ )
      {
         if( CPU_ISSET( idx, &threadcpu ) )
         {
            printf(" CPU-%d ", idx);
         }
      }
      printf( "\nLaunching thread %d\n", i );
      // Initializes the pointer passed in
      rc = pthread_attr_init( &rt_sched_attr[ i ] );
      // Sets attributes inherit from creating thread. Second parameter is the inherit scheduler.
      rc = pthread_attr_setinheritsched( &rt_sched_attr[ i ], PTHREAD_EXPLICIT_SCHED );
      // sets the scheduling policy attribute of the thread attributes object referred to by
      // attr to the value specified in policy. This attribute determines the scheduling policy of
      // a thread created using the thread attributes object attr.
      rc = pthread_attr_setschedpolicy( &rt_sched_attr[ i ], SCHED_FIFO );
      // The pthread_attr_setaffinity_np() function sets the CPU affinity mask attribute of the thread
      // attributes object referred to by attr to the value specified in cpuset. This attribute
      // determines the CPU affinity mask of a thread created using the thread attributes object attr.
      rc = pthread_attr_setaffinity_np( &rt_sched_attr[ i ], sizeof( cpu_set_t ), &threadcpu );
      // Setting thread priority, Thread 0 has highest priority, and each thread after has a priority
      // one less than the previous thread.
      rt_param[ i ].sched_priority = rt_max_prio - i - 1;
      // The pthread_attr_setschedparam() function sets the scheduling parameter attributes of the thread
      // attributes object referred to by attr to the values specified in the buffer pointed to by param.
      // These attributes determine the scheduling parameters of a thread created using the thread
      // attributes object attr.
      pthread_attr_setschedparam( &rt_sched_attr[ i ], &rt_param[ i ] );

      threadParams[ i ].threadIdx = i;

      threadParams[ i ].T = T[ i ] * NSEC_PER_MSEC;
      threadParams[ i ].C = C[ i ] * NSEC_PER_MSEC;

   }

   // The  pthread_join() function waits for the thread specified by thread to terminate. If that thread
   // has already terminated, then pthread_join() returns immediately. The thread specified by thread
   // must be joinable.
   uint32_t running = 1;
   struct timespec mainStartTime = { 0, 0 };
   struct timespec currentTime = { 0, 0 };
   struct timespec mainDelta = { 0, 0 };

   clock_gettime( CLOCK_REALTIME, &mainStartTime );
   for( i = 0; i < NUM_THREADS; i++ )
   {
      threadParams[ i ].previousRelease = mainStartTime;

      pthread_create( &threads[ i ],                  // pointer to thread descriptor
                      ( void *) 0,                    // use default attributes
                      counterThread,                  // thread function entry point
                      ( void *)&( threadParams[ i ] ) // parameters to pass in
                    );
      syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
               "Thread %d released iteration 1.\n", i );
      pthread_join( threads[ i ], NULL );
   }
   uint32_t iteration[ NUM_THREADS ] = { 1, 1, 1 };
   while( running )
   {
      clock_gettime( CLOCK_REALTIME, &currentTime );

      // The pthread_create() function starts a new thread in the calling process. The new thread starts
      // execution by invoking start_routine(); arg is passed as the sole argument of start_routine().
      for( i = 0; i < NUM_THREADS; i++ )
      {
         delta_t( &currentTime, &threadParams[ i ].previousRelease, &mainDelta );
         if( ( threadParams[ i ].T ) > mainDelta.tv_nsec )
         {
            delta_t( &currentTime, &mainStartTime, &mainDelta );
            syslog( LOG_MAKEPRI( LOG_USER, LOG_INFO ),
               "Thread %d released iteration %d after %d ms from start.\n",
               i, ++iteration[ i ], ( uint32_t ) mainDelta.tv_nsec / NSEC_PER_MSEC );

            threadParams[ i ].previousRelease = currentTime;
            pthread_create( &threads[ i ],                  // pointer to thread descriptor
                            ( void *) 0,                    // use default attributes
                            counterThread,                  // thread function entry point
                            ( void *)&( threadParams[ i ] ) // parameters to pass in
                           );
            pthread_join( threads[ i ], NULL );
         }
      }

      delta_t( &currentTime, &mainStartTime, &mainDelta );
      if( mainDelta.tv_sec > 1 )
      {
         running = 0;
      }
   }

   printf( "\nTEST COMPLETE\n" );
}
