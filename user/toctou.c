#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define SAFEFILE "toctou_safe"
#define SECRETFILE "toctou_secret"
#define TARGET "toctou_target"

static void
write_file(const char *path, const char *msg)
{
  int fd = open(path, O_CREATE | O_TRUNC | O_WRONLY);
  if(fd < 0){
    fprintf(2, "toctou: open %s failed\n", path);
    exit(1);
  }
  if(write(fd, msg, strlen(msg)) != (int)strlen(msg)){
    fprintf(2, "toctou: write %s failed\n", path);
    close(fd);
    exit(1);
  }
  close(fd);
}

int
main(void)
{
  struct stat checked;
  struct stat used;
  char buf[64];

  unlink(TARGET);
  unlink(SAFEFILE);
  unlink(SECRETFILE);

  write_file(SAFEFILE, "SAFE DATA\n");
  write_file(SECRETFILE, "SECRET DATA\n");

  if(link(SAFEFILE, TARGET) < 0){
    fprintf(2, "toctou: link %s -> %s failed\n", SAFEFILE, TARGET);
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "toctou: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // Attacker: swap the pathname after victim checks it.
    pause(10);
    unlink(TARGET);
    if(link(SECRETFILE, TARGET) < 0){
      fprintf(2, "toctou: attacker link failed\n");
      exit(1);
    }
    exit(0);
  }

  // Vulnerable check phase.
  if(stat(TARGET, &checked) < 0){
    fprintf(2, "toctou: stat(%s) failed\n", TARGET);
    exit(1);
  }

  printf("[victim] checked %s inode=%d\n", TARGET, checked.ino);

  // Time window between check and use.
  pause(30);

  int fd = open(TARGET, O_RDONLY);
  if(fd < 0){
    fprintf(2, "toctou: open(%s) failed\n", TARGET);
    exit(1);
  }

  if(fstat(fd, &used) < 0){
    fprintf(2, "toctou: fstat(%s) failed\n", TARGET);
    close(fd);
    exit(1);
  }

  int n = read(fd, buf, sizeof(buf) - 1);
  close(fd);
  if(n < 0){
    fprintf(2, "toctou: read(%s) failed\n", TARGET);
    exit(1);
  }
  buf[n] = '\0';

  wait(0);

  printf("[victim] opened %s inode=%d\n", TARGET, used.ino);
  printf("[victim] read content: %s", buf);

  if(checked.ino != used.ino){
    printf("[result] TOCTOU triggered: checked object != used object\n");
  } else {
    printf("[result] race did not trigger this run; run again\n");
  }

  exit(0);
}
