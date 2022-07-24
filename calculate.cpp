#include <pthread.h>
#include <assert.h>
#include <math.h>

#include "calculate.hpp"


/* thread main function */
static void * thread_entry_point(void * arg)
{
    ThreadArg * thread_arg = static_cast<ThreadArg *>(arg);
    double result = 0;

    double xi = thread_arg->start_x + (thread_arg->diff_x)/2;
    while(xi < thread_arg->end_x)
    {
        result += thread_arg->f(xi) * thread_arg->diff_x;
        xi += thread_arg->diff_x;
    }

    *(thread_arg->result) = result;
    return nullptr;
}

/* function for calculate integral sum */
static double calc_function(double x)
{
    extern double FUNCTION(double x);
    return FUNCTION(x);
}

/* API function for start concurrency calculate */
double result_parallel_calc(double x_start, double x_end, int cnt_threads)
{
    assert(x_start < x_end);

    /* 0) initialize handle structures */
    const double    ALL_RANGE   = x_end - x_start;
    const int       CNT_THREADS = cnt_threads;
    const double    RANGE       = ALL_RANGE / cnt_threads;
    const long      CNT_CUTS    = RANGE * 100'000;
    const double    EPS         = RANGE/CNT_CUTS;
    pthread_t       arr_tid[CNT_THREADS];
    ThreadArg       arr_arg[CNT_THREADS];
    double          arr_res[CNT_THREADS];

    /* 1) initialize thread arguments */
    for(int i = 0; i < CNT_THREADS; i++)
    {
        arr_arg[i].start_x  = x_start + i * RANGE;
        arr_arg[i].end_x    = x_start + (i+1) * RANGE;
        arr_arg[i].diff_x   = EPS;
        arr_arg[i].f        = calc_function;
        arr_arg[i].result   = &arr_res[i];
    }

    /* 2) start threads */
    for(int i = 0; i < CNT_THREADS; i++)
        assert(pthread_create(&arr_tid[i], NULL, thread_entry_point, &arr_arg[i]) == 0);

    /* 3) wait threads */
    for(int i = 0; i < CNT_THREADS; i++)
        assert(pthread_join(arr_tid[i], nullptr) == 0);

    /* 4) sum all results */
    double total_res = 0;
    for(int i = 0; i < CNT_THREADS; i++)
        total_res += arr_res[i];

    return total_res;
}
