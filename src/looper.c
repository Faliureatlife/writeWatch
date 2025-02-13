#include <sys/wait.h>

int run_comm(const char* comm, const char * location){
  printf("command function running:%s \n", comm);
  pid_t pid = fork();
  if (pid == 0) {
    FILE* output;
    output = freopen(location ,"w+", stdout);

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
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
    }
    return -1;
  } 
}

void handle_events(int fd, int *wd, int argc, char *argv[], int commpos){
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
      
      //only one the matters for (n)vim file writes
      //i did change something in my init.lua so not sure if that has a bearing on it
      if (event->mask & IN_CLOSE_WRITE)
        printf("IN_CLOSE_WRITE\n");

      //both watched for but not generated
      if (event->mask & IN_MOVED_TO)
        printf("IN_MOVED_TO\n");
      if (event->mask & IN_MOVED_FROM)
        printf("IN_MOVED_FROM\n");

      for (size_t i = 1; i < argc; ++i) {
        if (wd[i] == event->wd) {
          printf("%s/", argv[i]);
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

    if (commpos > 0) {
      run_comm(argv[commpos],"commandlog.txt");
    }
  }
}
