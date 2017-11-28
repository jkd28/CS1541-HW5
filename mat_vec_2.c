/****************************************************************************
*  Matrix vector multiplication using multiple threads
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_THREADS 32

int N;
float **a;
float *b;
float *x;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_argument {
    int colStart;
    int numColumns;
};


void *multiply_row(void *arg_struct){
    struct thread_argument *sent_args = arg_struct;
    int i, j;
    int start = sent_args->colStart;
    int endColumn = start + sent_args->numColumns;

    /* Perform multiplication */
    /*
        For each column, go through each row multiplying one element
        and storing it in the corresponding row in the result vector
    */
    for (j = start; j < endColumn; j++) {
        pthread_mutex_lock(&mutex);
        for (i = 0; i < N; i++) {
            x[i] += a[i][j] * b[j];
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main (int argc, char *argv[] )    {
    /* the array size should be supplied as a command line argument */
    if(argc != 3) {
        printf("USAGE: ./mat_vec_1 <N> <P>\n") ; exit(2) ;
    }
    N = atoi(argv[1]);
    int numThreads = atoi(argv[2]);
    printf("Array size = %d \n ", N );
    int mid = (N+1)/2;
    int i, j;
    double time_start, time_end;
    struct timeval tv;
    struct timezone tz;
    int colsPerThread, extraCols;

    /* allocate arrays dynamically */
    a = malloc(sizeof(float*)*N);
    for (i = 0; i < N; i++) {
        a[i] = malloc(sizeof(float)*N);
    }
    b = malloc(sizeof(float)*N);
    x = malloc(sizeof(float)*N);

    /* Inititialize matrix A and vector B. */
    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            if (j == i)                    { a[i][j] = 2.0; }
            else if (j == i-1 || j == i+1) { a[i][j] = 1.0; }
            else                           { a[i][j] = 0.01; }
        }
        b[i] = mid - abs(i-mid+1);
    }

    gettimeofday (&tv ,   &tz);
    time_start = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

    /* Inititialize positions in result array */
    for (i=0; i<N; i++) {
        x[i] = 0.0;
    }

    /* Inititialize thread/argument lists */
    pthread_t threads[MAX_THREADS];
    struct thread_argument arguments[MAX_THREADS];

    /* Determine number of rows to be handled by each thread */
    if (N % numThreads == 0) {
        colsPerThread = N / numThreads;
        extraCols = 0;
    } else {
        /* N is NOT evenly divisible by P */
        extraCols = N % numThreads;
        colsPerThread = (N - extraCols) / numThreads;
    }

    for (i = 0; i < numThreads; i++) {
        /* Build arguments for this particular thread */
        if (i == numThreads - 1) {
            arguments[i].numColumns= colsPerThread + extraCols;
        } else {
            arguments[i].numColumns = colsPerThread;
        }
        arguments[i].colStart = i*colsPerThread;

        pthread_create(&threads[i], NULL, multiply_row, &arguments[i]);
    }

    for (i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday (&tv ,  &tz);
    time_end = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

    /* this is for checking the results */
    for (i=0; i<N; i+=N/10) {
        printf(" %10.6f", i, x[i]);
    }
    printf("\n");
    printf ("time = %lf\n", time_end - time_start);

}
