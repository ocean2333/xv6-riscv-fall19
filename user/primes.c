#include "kernel/types.h"
#include "user/user.h"

void source() {
  int i;
  for (i = 2; i < 36; i++) {
    write(1, &i, sizeof(i));
  }
  close(1);
}

void cull(int p) {
  int n;
  while (read(0, &n, sizeof(n))) {
    if (n % p != 0) {
      write(1, &n, sizeof(n));
    }
  }
  exit(0);
}

void redirect(int k, int pd[]) {
  close(k);
  dup(pd[k]);
  close(pd[0]);
  close(pd[1]);
}

void sink() {
  int pd[2];
  int p;

  if (read(0, &p, sizeof(p))) {
    printf("prime %d\n", p);
    pipe(pd);
    int pid;
    while(-1 == (pid=fork())){
      ;
    }
    if (pid!=0) {
      redirect(0, pd);
      wait(0);
      sink();   
    } else {
      redirect(1, pd);
      cull(p);
    }
  }
}

int main(int argc, char *argv[]) {

  int pd[2];
  pipe(pd);

  if (fork()) {
    redirect(1, pd);
    source();
    wait(0);
  } else {
    redirect(0, pd);
    sink();
  }
  exit(0);
}