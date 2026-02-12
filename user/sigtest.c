#include "kernel/types.h"
#include "user/user.h"

volatile int got_usr1 = 0;

void
on_usr1(int signum)
{
  printf("child: handler got signal %d\n", signum);
  got_usr1 = 1;
  sigreturn();
}

int
main(void)
{
  int st, pid;

  pid = fork();
  if(pid < 0){
    fprintf(2, "sigtest: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    if(signal(SIGUSR1, on_usr1) < 0){
      fprintf(2, "sigtest: signal register failed\n");
      exit(1);
    }
    while(!got_usr1)
      pause(1);
    printf("child: survived SIGUSR1 with handler\n");
    exit(0);
  }

  pause(5);
  if(sigsend(pid, SIGUSR1) < 0){
    fprintf(2, "sigtest: sigsend(SIGUSR1) failed\n");
    exit(1);
  }
  wait(&st);
  printf("test1 exit status = %d (expect 0)\n", st);

  pid = fork();
  if(pid < 0){
    fprintf(2, "sigtest: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    while(1)
      pause(10);
  }

  pause(5);
  if(sigsend(pid, SIGUSR1) < 0){
    fprintf(2, "sigtest: sigsend(SIGUSR1 default) failed\n");
    exit(1);
  }
  wait(&st);
  printf("test2 exit status = %d (expect -1)\n", st);

  pid = fork();
  if(pid < 0){
    fprintf(2, "sigtest: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    if(signal(SIGUSR1, (void (*)(int))SIG_IGN) < 0){
      fprintf(2, "sigtest: signal ignore failed\n");
      exit(1);
    }
    pause(20);
    printf("child: ignored SIGUSR1\n");
    exit(0);
  }

  pause(5);
  if(sigsend(pid, SIGUSR1) < 0){
    fprintf(2, "sigtest: sigsend(SIGUSR1 ignore) failed\n");
    exit(1);
  }
  wait(&st);
  printf("test3 exit status = %d (expect 0)\n", st);

  exit(0);
}
