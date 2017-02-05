//------------------------------------------------------------------------------
// ECEN 5623 - Spring 2017
//   Exercise 1
//   Tyler Kidney & Robert Blazewicz
//------------------------------------------------------------------------------

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>

#define XXX_TASKS       ( 3 )
#define XXX_SPIN_COUNT  ( 1000000 )

#define SCHED_POLICY    ( SCHED_FIFO )
#define CbTRUE          ( 1 )
#define CbFALSE         ( 0 )

//------------------------------------------------------------------------------

extern void *ProcXXX_Task1( void *threadp );
extern void *ProcXXX_Task2( void *threadp );
extern void *ProcXXX_Task3( void *threadp );

//------------------------------------------------------------------------------

typedef uint8_t TbBOOLEAN;
typedef uint8_t BYTE;

typedef struct TeXXX_h_ThreadParams
{
   uint32_t e_Idx_Thread;
   TbBOOLEAN e_b_IncrementFlag;

//TODO add T
//TODO add C
} TeXXX_h_ThreadParams;

typedef void *( *TeXXX_ptr_Tasks )( void *threadp );

//------------------------------------------------------------------------------

static TeXXX_ptr_Tasks VaXXX_ptr_Tasks[ XXX_TASKS ] =
{
   ProcXXX_Task1,
   ProcXXX_Task1,
   ProcXXX_Task1
};

//------------------------------------------------------------------------------

// POSIX thread declarations and scheduling attributes
pthread_attr_t rt_sched_attr;
pthread_t VaXXX_h_PThreads[ XXX_TASKS ];
TeXXX_h_ThreadParams VaXXX_h_ThreadParams[ XXX_TASKS ];

uint32_t rt_max_prio, rt_min_prio;
struct sched_param rt_param;
struct sched_param nrt_param;
struct sched_param tstThread_param;
pthread_attr_t tstThread_sched_attr;

sem_t startIOWorker[XXX_TASKS];

// Unsafe global
//int VeXXX_Cnt_Sum;

//------------------------------------------------------------------------------

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

void setSchedPolicy( void )
{
   uint32_t rc, scope;

   print_scheduler( );

   pthread_attr_init( &rt_sched_attr );
   pthread_attr_init( &tstThread_sched_attr );
   pthread_attr_setinheritsched( &rt_sched_attr, PTHREAD_EXPLICIT_SCHED );
   pthread_attr_setschedpolicy( &rt_sched_attr, SCHED_POLICY );
   pthread_attr_setinheritsched( &tstThread_sched_attr, PTHREAD_EXPLICIT_SCHED );
   pthread_attr_setschedpolicy( &tstThread_sched_attr, SCHED_POLICY );

   rt_max_prio = sched_get_priority_max( SCHED_POLICY );
   rt_min_prio = sched_get_priority_min( SCHED_POLICY );

   rc=sched_getparam( getpid( ), &nrt_param );
   rt_param.sched_priority = rt_max_prio;

   //#ifdef SET_SCHED_POLICY
   rc = sched_setscheduler( getpid( ), SCHED_POLICY, &rt_param );

   if( rc )
   {
      printf( "ERROR: sched_setscheduler rc is %d\n", rc );
      perror( "sched_setscheduler" );
   }
   else
   {
      printf( "SCHED_POLICY SET: sched_setscheduler rc is %d\n", rc );
   }

   print_scheduler( );
   //#endif

   printf( "min prio = %d, max prio = %d\n", rt_min_prio, rt_max_prio );
   pthread_attr_getscope( &rt_sched_attr, &scope );

   if ( scope == PTHREAD_SCOPE_SYSTEM )
   {
      printf( "PTHREAD SCOPE SYSTEM\n" );
   }
   else if ( scope == PTHREAD_SCOPE_PROCESS )
   {
      printf( "PTHREAD SCOPE PROCESS\n" );
   }
   else
   {
      printf( "PTHREAD SCOPE UNKNOWN\n" );
   }

   rt_param.sched_priority = rt_max_prio - 1;
   pthread_attr_setschedparam( &rt_sched_attr, &rt_param );
   tstThread_param.sched_priority = rt_max_prio;
   pthread_attr_setschedparam( &tstThread_sched_attr, &tstThread_param );
}

//------------------------------------------------------------------------------

void *ProcXXX_Task1( void *threadp )
{
   TeXXX_h_ThreadParams *VaXXX_h_ThreadParams = ( TeXXX_h_ThreadParams* )threadp;

   sem_wait( &( startIOWorker[ VaXXX_h_ThreadParams->e_Idx_Thread ] ) );

   printf( "Thread %d starting...\n", VaXXX_h_ThreadParams->e_Idx_Thread );
   // Change printf to dmsg

   // get start time
   // do until elapse time
   //   spin for fixed interval (test time elasped?)
   //   sleep until start+X

   printf( "Thread %d stopping...\n", VaXXX_h_ThreadParams->e_Idx_Thread );

   pthread_exit( threadp );
}

//------------------------------------------------------------------------------

uint32_t main( uint32_t argc, uint8_t *argv[ ] )
{
   uint32_t i;

   //  VeXXX_Cnt_Sum = 0;

   // Startup
   setSchedPolicy( );

   //TODO Need to set processor affinity all to processor 1

   //TODO Set decreasing priority for each task

   // Create the tasks
   for( i = 0; i < XXX_TASKS; i++ )
   {
      if ( sem_init (&(startIOWorker[i]), 0, 0 ) )
      {
         printf( "Failed to initialize startIOWorker semaphore %d\n", i );
         exit( -1 );
      }

      VaXXX_h_ThreadParams[ i ].e_Idx_Thread = i;
      VaXXX_h_ThreadParams[ i ].e_b_IncrementFlag = ( ( i % 2 ) == 0 ) ? CbTRUE : CbFALSE;
      pthread_create( &VaXXX_h_PThreads[i],
                      ( void *)0 /* &tstThread_sched_attr */,
                      VaXXX_ptr_Tasks[ i ],
                      ( void *)&( VaXXX_h_ThreadParams[ i ] ) );
   }

   printf( "\nPress key to continue" );
   getchar( );

   // Unblock the tasks
   for( i = 0; i < XXX_TASKS; i++ )
   {
      sem_post( &( startIOWorker[ i ] ) );
   }

   // Wait for the tasks to terminate
   for( i = 0; i < XXX_TASKS; i++ )
   {
      pthread_join( VaXXX_h_PThreads[ i ], NULL );
   }

   // Shutdown
   for( i = 0; i < XXX_TASKS; i++ )
   {
      sem_destroy( &( startIOWorker[ i ] ) );
   }

   printf( "TEST COMPLETE\n" );
}
