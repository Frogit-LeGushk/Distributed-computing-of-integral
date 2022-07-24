#pragma once

/* argument for thread entry as 'C' structure */
struct ThreadArg {
    double      start_x         ;
    double      end_x           ;
    double      diff_x          ;
    double      (*f)(double x)  ;
    double *    result          ;
};

/* API function for start concurrency calculate */
extern double result_parallel_calc(double x_start, double x_end, int cnt_threads);





