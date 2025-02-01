#include <errno.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>

void exec_code(FILE* handle, const char* command){

}
                                                        //,const char* COMMAND
void handle_events(int fd, int *wd, int argc, char *argv[]){
  //need to ensure that the buffer is aligned 
  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t len;

  //read all file descriptors
  for(;;) {
    len = read(fd,buf,sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      perror("Error in read");
      exit(EXIT_FAILURE);
    }
    
    if (len <= 0) break;

    for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event*) ptr;

      if (event->mask & IN_MODIFY) 
        printf("IN_MODIFY\n");
      if (event->mask & IN_CLOSE_NOWRITE)
        printf("IN_CLOSE\n");
      if (event->mask & IN_CLOSE_WRITE)
        printf("IN_CLOSE_WRITE\n");
      if (event->mask & IN_MOVED_TO)
        printf("IN_MOVED_TO\n");
      if (event->mask & IN_MOVED_FROM)
        printf("IN_MOVED_FROM\n");
      if (event->mask & IN_OPEN)
        printf("IN_OPEN\n");

      for (size_t i = 1; i < argc; ++i) {
        if (wd[i] == event->wd) {
          printf("%s/n", argv[i]);
          break;
        }
      }

      //event_len only exists if it has a name
      if(event->len)
        printf("%s", event->name);

      if(event->mask & IN_ISDIR)
        printf(" [directory]\n");
      else 
        printf(" [file]\n\n");

    }
  }
}

int main(int argc, char *argv[]){
  char buf;
  int fd, i, poll_num;
  int *wd;
  nfds_t nfds;
  struct pollfd fds[2];
  
  //ensure args are passed
  fprintf(stderr, "%s\n",argv[0]);
  //Unimplemented things
  if (argc < 2) {
    printf("Usage: %s [FILE] PATH [OPTION]\nArgs:\n\t -c COMMAND\n\n\t -f FILE: Execute command stored in file\n\n\t -o FILE: Choose file to display information out to, stdout by default\n\n\t -d: Daemonize the instance\n\n\t --help: Display this message\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  //create file descriptor and ensure successful creation
  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {perror("Init error; inotify_inti1"); exit(EXIT_FAILURE);}

  //calloc is like malloc but initialzies to 0 
  //allocates (argc * sizeof(int)) bytes;
  //allocating the watch descriptors
  wd = calloc(argc, sizeof(int));
  if (wd == NULL) {perror("Init error; calloc"); exit(EXIT_FAILURE);}

  //add directories into the watch list
  //we are watching for file close and for file modify
  for (i = 1; i < argc; i++) {
    //IN_MODIFY doesnt seem to really work for VIM
    wd[i] = inotify_add_watch(fd, argv[i], IN_CLOSE | IN_MOVE);
    //ensure add success
    if(wd[i] == -1) {
      perror("unable to add one or more files to watch");
      exit(EXIT_FAILURE);
    }
  }

  //rewrite to only bother reading the first into file
  
  //basically just a long int for some safety
  nfds = 2;

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  fds[1].fd = fd;
  fds[1].events = POLLIN;

  printf("Listening for events on %s", argv[1]);
  
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
      //if there is something that has happened (revents = 1) then we call the event handler
      if (fds[0].revents & POLLIN) {
        while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n') continue; 
        break;
      }
      if (fds[1].revents & POLLIN) {
        handle_events(fd,wd,argc,argv);
      }
    }
  }
  
  printf("Exiting program \n");

  //free up memory and handles
  close(fd);
  free(wd);

  exit(EXIT_SUCCESS);
}
