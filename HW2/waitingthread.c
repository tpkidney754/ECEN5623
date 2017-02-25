#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SEM_NAME      "mysemaphore"

sem_t *semname;

int main( int argc, char** argv )
{
   semname = sem_open( SEM_NAME, O_CREAT, 0644, 0 );

   printf( "Creating waiting process\n" );
   printf( "Waiting on semaphore\n" );
   sem_wait( semname );
   printf( "No longer waiting on semaphore\n" );
   sem_destroy( semname );
   exit( EXIT_SUCCESS );
}
