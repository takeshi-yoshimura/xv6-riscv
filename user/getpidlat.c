#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int iters = 200000;
  if(argc > 1){
    iters = atoi(argv[1]);
  }
  if(iters <= 0){
    fprintf(2, "usage: getpidlat [iters > 0]\n");
    exit(1);
  }

  // Warm up instruction/data paths before timing.
  for(int i = 0; i < 1000; i++){
    (void)getpid();
  }

  int t0 = uptime();
  for(int i = 0; i < iters; i++){
    (void)getpid();
  }
  int t1 = uptime();

  int dt = t1 - t0;
  if(dt <= 0){
    printf("getpidlat: elapsed ticks too small (%d). Try more iterations.\n", dt);
    exit(0);
  }

  uint64 calls_per_tick = (uint64)iters / (uint64)dt;
  uint64 rem = (uint64)iters % (uint64)dt;
  uint64 calls_per_tick_frac = (rem * 100ULL) / (uint64)dt;

  printf("getpid latency benchmark\n");
  printf("  iterations: %d\n", iters);
  printf("  elapsed ticks: %d\n", dt);
  if(calls_per_tick_frac < 10){
    printf("  throughput: %d.0%d getpid/tick\n",
           (int)calls_per_tick, (int)calls_per_tick_frac);
    printf("  avg latency: ~1/%d.0%d tick per getpid\n",
           (int)calls_per_tick, (int)calls_per_tick_frac);
  } else {
    printf("  throughput: %d.%d getpid/tick\n",
           (int)calls_per_tick, (int)calls_per_tick_frac);
    printf("  avg latency: ~1/%d.%d tick per getpid\n",
           (int)calls_per_tick, (int)calls_per_tick_frac);
  }

  exit(0);
}
