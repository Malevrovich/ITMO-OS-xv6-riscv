#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int p[2][2];
  char to_child_msg[] = "ping";
  char to_parent_msg[] = "pong";

  if(pipe(p[0]) >= 0){
    if(pipe(p[1]) >= 0){
      int pid = fork();

      if(pid == 0){
        close(p[0][1]);
        close(p[1][0]);

        for(uint64 i = 0; i < sizeof(to_child_msg); i++){
          to_child_msg[i] -= 1;
        }
        if(read(p[0][0], to_child_msg, sizeof(to_child_msg)) < sizeof(to_child_msg)){
          printf("Not enough bytes were read in %d\n", getpid());
          exit(-1);
        }

        close(p[0][0]);
        printf("%d: got %s\n", getpid(), to_child_msg);

        if(write(p[1][1], to_parent_msg, sizeof(to_parent_msg)) < sizeof(to_parent_msg)){
          printf("Not enough bytes were written in %d\n", getpid());
          exit(-1);
        }
        close(p[1][1]);
      } else if(pid > 0){
        close(p[0][0]);
        close(p[1][1]);

        if(write(p[0][1], to_child_msg, sizeof(to_child_msg)) < sizeof(to_child_msg)){
          printf("Not enough bytes were written in %d\n", getpid());
          exit(-1);
        }
        close(p[0][1]);

        for(uint64 i = 0; i < sizeof(to_parent_msg); i++){
          to_parent_msg[i] -= 1;
        }
        if(read(p[1][0], to_parent_msg, sizeof(to_parent_msg)) < sizeof(to_parent_msg)){
          exit(-1);
        }
        close(p[1][0]);

        printf("%d: got %s\n", getpid(), to_parent_msg);
      } else {
        printf("Fork failed\n");
        exit(-1);
      }

      exit(0);
    } else {
      printf("Second pipe failed\n");
      close(p[0][1]);
      close(p[0][0]);
      exit(-1);
    }
  }
  printf("First pipe failed\n");
  exit(-1);
  return -1;
}