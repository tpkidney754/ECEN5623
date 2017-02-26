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
} Attitude_t;

Attitude_t attitude;
pthread_mutex_t attitudeMut = PTHREAD_MUTEX_INITIALIZER;

void UpdateAttitude( );
void ReadAttitude( );

static void ( *ThreadFunctions[ NUM_THREADS ] ) =
              { UpdateAttitude,
                ReadAttitude };

int main( int argc, char** argv )
{
   pthread_t threads[ NUM_THREADS ];
   int thread_args[ NUM_THREADS ];
   int result_code;
   unsigned index;

   attitude.x            = 0;
   attitude.y            = 0;
   attitude.z            = 0;
   attitude.acceleration = 0;
   attitude.roll         = 0;
   attitude.pitch        = 0;
   attitude.yaw          = 0;

   // create all threads one by one
   for( index = 0; index < NUM_THREADS; ++index )
   {
      thread_args[ index ] = index;
      printf("In main: creating thread %d\n", index);
      result_code = pthread_create( &threads[ index ],
                                    NULL,
                                    ThreadFunctions[ index ],
                                    &thread_args[index] );
      assert( !result_code );
   }

   // wait for each thread to complete
   for( index = 0; index < NUM_THREADS; ++index )
   {
      // block until thread 'index' completes
      result_code = pthread_join( threads[ index ], NULL );
      assert( !result_code );
      printf( "In main: thread %d has completed\n", index );
   }

   printf( "In main: All threads completed successfully\n" );
   exit( EXIT_SUCCESS );
}

void UpdateAttitude( void* argument )
{
   printf( "Attitude data is being updated\n" );
   clockid_t clock;
   // Locking the access to the attitude structure
   pthread_mutex_lock( &attitudeMut );
   // The sleep will help show that the mutex is working.
   // Thread 2 that reads is running is waiting for the mutex to unlock
   // If the mutex is not used, then the ReadAttitude thread always reads 0's
   sleep( 1 );
   // Getting the time when the data is updated and is used to create random numbers.
   clock_gettime( clock, &attitude.sampleTime );
   srand( ( unsigned ) attitude.sampleTime.tv_nsec );
q
   attitude.x            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.y            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.z            = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.acceleration = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.roll         = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.pitch        = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   attitude.yaw          = rand( ) + ( ( double ) rand( ) / ( double ) RAND_MAX );
   pthread_mutex_unlock( &attitudeMut );
}

void ReadAttitude( )
{
   printf( "Reading Attitude data\n" );
   pthread_mutex_lock( &attitudeMut );
   printf( "Attitude data:\n \
            x            = %lf\n \
            y            = %lf\n \
            z            = %lf\n \
            acceleration = %lf\n \
            roll         = %lf\n \
            pitch        = %lf\n \
            yaw          = %lf\n",
            attitude.x, attitude.y, attitude.z, attitude.acceleration,
            attitude.roll, attitude.pitch, attitude.yaw );
   pthread_mutex_unlock( &attitudeMut );
}
