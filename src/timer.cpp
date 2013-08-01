#include "timer.hpp"

Timer::Timer()
{
  Reset();
}

Timer::~Timer()
{
}

void Timer::Reset()
{
  gettimeofday(&start, 0);
}

int64_t Timer::ElapsedMilliseconds()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  int64_t secs = tv.tv_sec - start.tv_sec;
  int64_t usecs = tv.tv_usec - start.tv_usec;
  int64_t msecs = (secs * 1000) + ((usecs / 1000.0) + 0.5);
  return msecs;
}
