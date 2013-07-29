#include <stdint.h>
#include <sys/time.h>

class Timer
{
public:
  Timer();
  virtual ~Timer();

  void Reset();
  int64_t ElapsedMilliseconds();

protected:
  struct timeval start;
};
