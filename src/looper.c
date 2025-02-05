#include "opts.c"


//stdout held in STDOUT_FILENO
//returns old stdout
int redirect_output(FILE* new_location){
  int old_std = dup(STDOUT_FILENO);

  //fileno gives file descriptor of stream
  dup2(fileno(new_location),STDOUT_FILENO);

  return old_std;
}

void run_comm(const char * comm, FILE* location){
    pid_t pid = fork();
  if (pid == 0) {
    FILE* output;

       // output = freopen(location ,"w", stdout);

    if (output == NULL){
      perror("No or unavailible output file");
      exit(EXIT_FAILURE);
    }
    //use sh to execute our command 
    execlp("sh", "sh", "-c", comm, NULL);

    perror("Unable to execute command");
    exit(EXIT_FAILURE);
  } else {
    int status;
    waidpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
    }
    return -1;
  } 
}

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
