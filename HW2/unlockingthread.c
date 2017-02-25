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
   semname = sem_open( SEM_NAME, 0 );

   printf( "Creating unlocking process\n" );
   sem_post( semname );
   printf( "Semaphore has been unlocked\n" );
   exit( EXIT_SUCCESS );
}
