#include <errno.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "looper.c"
#include "init.c"

//FIND SOMEWHERE TO PUT THESE
//stdout held in STDOUT_FILENO
//returns old stdout
int redirect_output(int fd){
  int old_std = dup(STDOUT_FILENO);

  //fileno gives file descriptor of stream
  dup2(fd,STDOUT_FILENO);

  return old_std;
}

//FIND SOMEWHERE TO PUT THESE
void restore_std(int stdoutNO){
  dup2(stdoutNO, STDOUT_FILENO);
}

int main(int argc, char *argv[]){
  char buf;
  int fd, i, poll_num, fdo, old_std, commpos;
  int *wd;
  nfds_t nfds;
  struct pollfd fds[2];
  
  //ensure args are passed
  fprintf(stderr, "%s\n",argv[0]);

  if (argc < 2) {
    printf("Usage: %s [FILE] PATH [OPTION]\nArgs:\n\t -c COMMAND\n\n\t -f FILE: Execute command stored in file\n\n\t -o FILE: Choose file to display information out to, stdout by default\n\n\t -d: Daemonize the instance\n\n\t --help: Display this message\n\nBy default the results of commands will be piped into $PWD/commandlog.txt", argv[0]);
    exit(EXIT_FAILURE);
  }

  //calloc is like malloc but initialzies to 0 
  //allocates (argc * sizeof(int)) bytes;
  //allocating the watch descriptors
  wd = calloc(argc, sizeof *wd);
  if (wd == NULL) {perror("Init error; calloc"); exit(EXIT_FAILURE);}

  //for some reason I'm unsure about using &
  init_inotify(&argc,argv, &fd, &wd, &nfds, &fds);
  printf("Listening for events on %s\nPress enter to stop watching \n\n", argv[1]);

  //parsing arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp("-o",argv[i]))
      old_std = redirect_output(fdo = open(argv[i+1], O_WRONLY | O_APPEND | O_CREAT,0644));

    // if (!strcmp("-f",argv[i]))
    //
    if (!strcmp("-c",argv[i]))
      commpos = i + 1;

    if (!strcmp("-d",argv[i])) {
      daemonize("/dev/null");
      store_pid();
    }
  }

  //not sure if this should go here or in the loop
  if (signal(SIGTERM,sig_handler) == SIG_ERR){
    syslog(LOG_ERR, "Can't catch SIGTERM");
    exit(EXIT_FAILURE);
  }

  //loop section 
  for(;;) {
    //poll to fds, for nfds files, and never timeout
    poll_num = poll(fds,nfds,-1);
    //ensure poll created
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      exit(EXIT_FAILURE);
    } 

    if (poll_num > 0) {
      //if there is something that has happened (revents != 0) then we call the event handler
      if (fds[0].revents & POLLIN) {
        while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n') continue; 
        break;
      }
      if (fds[1].revents & POLLIN) {
        handle_events(fd,wd,argc,argv,commpos);
      }
    }
  }

  //free up memory and handles
  restore_std(old_std);
  printf("Exiting program \n");

  close(fdo);
  close(fd);
  free(wd);

  exit(EXIT_SUCCESS);
}
