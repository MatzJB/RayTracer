
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include <sys/time.h>//must have this

double csecond();

double csecond()
{
  struct timeval  tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec * 1.0e-6;
}


