#include <math.h>
#include <stdio.h>
#include <stdint.h>

#define TRUE            1
#define FALSE           0

#define NUM_EXAMPLES    9
#define MAX_TASKS       5

//-----------------------------------------------------------------------------

uint32_t period[ NUM_EXAMPLES ][ MAX_TASKS ] =
  {
    { 2, 10, 15 },
    { 2, 5, 7 },
    { 2, 5, 7, 13 },
    { 3, 5, 15 },
    { 2, 4, 16 },
    { 2, 5, 10 },
    { 2, 5, 7, 13 },
    { 3, 5, 15 },
    { 2, 5, 7, 13 }
  };

uint32_t wcet[ NUM_EXAMPLES ][ MAX_TASKS ] =
  {
    { 1, 1, 2 },
    { 1, 1, 2 },
    { 1, 1, 1, 2 },
    { 1, 2, 3 },
    { 1, 1, 4 },
    { 1, 2, 1 },
    { 1, 1, 1, 2 },
    { 1, 2, 4 },
    { 1, 1, 1, 2 }
  };

uint32_t numTasks[ NUM_EXAMPLES ] = { 3, 3, 4, 3, 3, 3, 4, 3, 4 };
float utilization[ NUM_EXAMPLES ];

//-----------------------------------------------------------------------------

int32_t completion_time_feasibility( uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline );
int32_t scheduling_point_feasibility( uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline );
int32_t dm_feasibility( uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline );
int32_t edf_feasibility( uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline );
int32_t llf_feasibility( uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline );

//-----------------------------------------------------------------------------

int32_t main( void )
  {
  for ( uint32_t i = 0; i < NUM_EXAMPLES; i++ )
    {
    utilization[ i ] = 0;
    for ( uint32_t j = 0; j < numTasks[ i ]; j++ )
      {
      utilization[ i ] += 1.0 * wcet[ i ][ j ]/period[ i ][ j ];
      }

    printf("Ex-%d U = %.4f: ", i, utilization[ i ] );
    for ( uint32_t j = 0; j < numTasks[ i ]; j++ )
      {
      printf("C%d = %d T%d = %d", j + 1, wcet[ i ][ j ], j + 1, period[ i ][ j ] );
      if (j < numTasks[i] - 1)
        {
        printf(", ");
        }
      }
    printf( "\n" );
    printf("  Completion Test is%sFeasible\n",
           completion_time_feasibility( numTasks[ i ], period[ i ], wcet[ i ], period[ i ] ) ? " " : " not " );
    printf("  Scheduling Point is%sFeasible\n",
           scheduling_point_feasibility( numTasks[ i ], period[ i ], wcet[ i ], period[ i ] ) ? " " : " not " );
    printf("  DM is%sFeasible\n",
           dm_feasibility( numTasks[ i ], period[ i ], wcet[ i ], period[ i ] ) ? " " : " not " );
    printf("  EDF is%sFeasible\n",
           edf_feasibility( numTasks[ i ], period[ i ], wcet[ i ], period[ i ] ) ? " " : " not " );
    //printf("  LLF is%sFeasible\n",
    //       llf_feasibility( numTasks[ i ], period[ i ], wcet[ i ], period[ i ] ) ? " " : " not " );
    if (i < NUM_EXAMPLES - 1)
      {
      printf("\n\n");
      }
    }
  }

//-----------------------------------------------------------------------------

int32_t completion_time_feasibility( uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline )
  {
  int32_t i, j;
  uint32_t an, anext;

  // Assume feasible until we find otherwise
  int32_t set_feasible = TRUE;

  //printf("numServices=%d\n", numServices);

  for ( i = 0; i < numServices; i++ )
    {
    an = 0; anext = 0;

    for ( j = 0; j <= i; j++ )
      {
      an += wcet[j];
      }

    while ( 1 )
      {
      anext = wcet[ i ];

      for ( j = 0; j < i; j++ )
        {
        anext += ceil( ( ( double )an ) / ( ( double )period[ j ] ) ) * wcet[ j ];
        }

      if ( anext == an )
        {
        break;
        }
      else
        {
        an = anext;
        }
      }

    if ( an > deadline[ i ] )
      {
      set_feasible = FALSE;
      }
    }

  return set_feasible;
  }

//-----------------------------------------------------------------------------

int32_t scheduling_point_feasibility(uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline )
  {
  int32_t rc = TRUE, i, j, k, l, status, temp;

  for ( i = 0; i < numServices; i++ ) // iterate from highest to lowest priority
    {
    status = 0;

    for ( k = 0; k <= i; k++ )
      {
      for ( l = 1; l <= ( floor( ( double ) period[ i ] / ( double ) period[ k ] ) ); l++)
        {
        temp = 0;

        for ( j = 0; j <= i; j++ )
          {
          temp += wcet[j] * ceil( ( double ) l * ( double ) period[ k ] / ( double ) period[ j ] );
          }

        if ( temp <= ( l * period[ k ] ) )
          {
          status = 1;
          break;
          }
        }

      if ( status )
        {
        break;
        }
      }

    if ( !status )
      {
      rc = FALSE;
      }
    }

  return rc;
  }

//-----------------------------------------------------------------------------

int32_t dm_feasibility(uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline )
  {
  int32_t rc = FALSE, i;
  double temp;

  // sum(Ci/Ti) <= 1
  temp = 0;
  for ( i = 0; i < numServices; i++ )
    {
    temp += ( double ) wcet[ i ] / ( double ) period[ i ];
    //printf("    %f = %d / %d\n", temp, wcet[i], period[i]);
    }
  if (temp <= 1.0)
    {
    rc = TRUE;
    }

  return rc;
  }

//-----------------------------------------------------------------------------

int32_t edf_feasibility(uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline )
  {
  int32_t rc = FALSE, i;
  double temp;

  // sum(Ci/Di) <= 1
  temp = 0;
  for ( i = 0; i < numServices; i++ )
    {
    temp += ( double ) wcet[ i ] / ( double ) deadline[ i ];
    //printf("    %f = %d / %d\n", temp, wcet[i], deadline[i]);
    }
  if (temp <= 1.0)
    {
    rc = TRUE;
    }

  return rc;
  }

//-----------------------------------------------------------------------------

int32_t llf_feasibility(uint32_t numServices, uint32_t *period, uint32_t *wcet, uint32_t *deadline )
  {
  int32_t rc = TRUE;

  rc = FALSE;

  return rc;
  }
