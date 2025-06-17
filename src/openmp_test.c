
/*
Openmp test, testing difference in scheduling
-Matz JB
*/

#include <unistd.h>

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <omp.h>


#define NUM_LOOPS 20
#define N_THREADS 2


double csecond();

int main( ) 
{

  int threads_workload[N_THREADS];//contain the number of times a thread is doing work

    printf("testing openmp...\n");
   
    /*
   ***: parallelising for loops with varying work timings and scheduling.
   
   Motive: Examine the different scheduling techniques in openmp.
    */
    
    double t = csecond();
    
    /* Set no. of threads dynamically. */
    omp_set_num_threads(N_THREADS);
    
    /* Max number of procs and threads. */
    printf("num_procs   = %d\n", omp_get_num_procs());
    printf("max_threads = %d\n", omp_get_max_threads());
    
    omp_set_num_threads(N_THREADS);
    
    
    for(int i=0; i<N_THREADS; i++)
      {
	threads_workload[i]=0;
      }

#pragma omp parallel
    {
#pragma omp for schedule(static, 1)
      for (int i=0; i<NUM_LOOPS; i++) 
        {
	  
	  sleep(i%N_THREADS);//0<-[0, N_THREADS-1]
	  threads_workload[omp_get_thread_num()]++;
	  
	  
	} //for
    }
      t = csecond()-t;

      printf("Elapsed time: %1.1f s\n", t);

      t=0;


    for(int i=0; i<N_THREADS; i++)
      {
	threads_workload[i]=0;
      }

#pragma omp parallel
    {
#pragma omp for schedule(dynamic, 2)
      for (int i=0; i<NUM_LOOPS; i++) 
        {
	  sleep(i%N_THREADS);//0<-[0, N_THREADS-1]
	  threads_workload[omp_get_thread_num()]++;
	  
	  
	} //for
    }
      t = csecond()-t;

      printf("Elapsed time: %1.1f s\n", t);

      printf("thread workload:\n");     
      for(int i=0; i<N_THREADS; i++)
	printf(" %d", threads_workload[i]);
      printf("\n");

      /*
        #pragma omp for schedule(static, STATIC_CHUNK)
        for (int i = 0 ; i < NUM_LOOPS ; ++i) 
        {
            if ((i % SLEEP_EVERY_N) == SLEEP_EVERY_N) 
                Sleep(0);
            nStaticN[i] = omp_get_thread_num( );
        }

        #pragma omp for schedule(dynamic, 1)
        for (int i = 0 ; i < NUM_LOOPS ; ++i) 
        {
            if ((i % SLEEP_EVERY_N) == SLEEP_EVERY_N) 
                Sleep(0);
            nDynamic1[i] = omp_get_thread_num( );
        }

        #pragma omp for schedule(dynamic, DYNAMIC_CHUNK)
        for (int i = 0 ; i < NUM_LOOPS ; ++i) 
        {
            if ((i % SLEEP_EVERY_N) == SLEEP_EVERY_N) 
                Sleep(0);
            nDynamicN[i] = omp_get_thread_num( );
        }

        #pragma omp for schedule(guided)
        for (int i = 0 ; i < NUM_LOOPS ; ++i) 
        {
            if ((i % SLEEP_EVERY_N) == SLEEP_EVERY_N) 
                Sleep(0);
            nGuided[i] = omp_get_thread_num( );
        }
	    */


	/*
    printf_s("------------------------------------------------\n");
    printf_s("| static | static | dynamic | dynamic | guided |\n");
    printf_s("|    1   |    %d   |    1    |    %d    |        |\n",
             STATIC_CHUNK, DYNAMIC_CHUNK);
    printf_s("------------------------------------------------\n");



    for (int i=0; i<NUM_LOOPS; ++i) 
    {
        printf_s("|    %d   |    %d   |    %d    |    %d    |"
                 "    %d   |\n",
                 nStatic1[i], nStaticN[i],
                 nDynamic1[i], nDynamicN[i], nGuided[i]);
    }

    printf_s("------------------------------------------------\n");
	*/


}
