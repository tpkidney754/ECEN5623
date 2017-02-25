#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <assert.h>
#include <unistd.h>

#define NUM_THREADS     2

int SetupSemaphore( sem_t * semname );
void* perform_work( void* argument );

sem_t semname;

int main( int argc, char** argv )
{
   pthread_t threads[ NUM_THREADS ];
   int thread_args[ NUM_THREADS ];
   int result_code;
   unsigned index;

   SetupSemaphore( &semname );
   // create all threads one by one
   for( index = 0; index < NUM_THREADS; ++index )
   {
      thread_args[ index ] = index;
      printf("In main: creating thread %d\n", index);
      result_code = pthread_create( &threads[index], NULL, perform_work, &thread_args[index] );
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

void* perform_work( void* argument )
{
   int passed_in_value;

   passed_in_value = *( ( int* )argument );

   if( passed_in_value == 0 )
   {
      sem_wait( &semname );
      printf( "Hello World! It's me, thread with argument %d!\n", passed_in_value );
   }
   else
   {
      printf( "Hello World! It's me, thread with argument %d!\n", passed_in_value );
      printf( "Incrementing semaphore from thread %d in ", passed_in_value );
      for( int i = 5; i >= 0; i-- )
      {
         printf( "%d..", i );
         sleep( 1 );
      }
      printf( "\n" );
      sem_post( &semname );
   }

   return NULL;
}

int SetupSemaphore( sem_t * semname )
{
   sem_init( semname, 0, 0 );
}
