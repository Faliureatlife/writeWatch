
void init_inotify(int* argc, char* argv[], int *fd, int** wd, nfds_t* nfds, struct pollfd (*fds)[2]){
                                                              // using (*fds)[2] which is pointer to array of two pollfd
                                                              // if i used *fds[2] it would be array to two pointers  
  int i;

  //create file descriptor and ensure successful creation
  *fd = inotify_init1(IN_NONBLOCK);
  if (*fd == -1) {perror("Init error; inotify_inti1"); exit(EXIT_FAILURE);}

  //add directories into the watch list
  //we are watching for file close and for file modify
  //most likely should ignore IN_ACCESS, not sure what others to watch
  //vim only consistenly generates IN_CLOSE
  (*wd)[1] = inotify_add_watch(*fd, argv[1], IN_CLOSE_WRITE | IN_MOVE);
  //ensure add success
  if((*wd)[1] == -1) {
    perror("unable to add one or more files to watch");
    exit(EXIT_FAILURE);
  }

  //basically just a long int for some safety
  *nfds = 2;

  (*fds)[0].fd = STDIN_FILENO;
  (*fds)[0].events = POLLIN;

  (*fds)[1].fd = *fd;
  (*fds)[1].events = POLLIN;
}

#define PID_FILE "/tmp/watcherDaemon.pid"

void store_pid() {
  char pid[32];
  int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    syslog(LOG_ERR, "Failed to create PID file");
    exit(EXIT_FAILURE);
  }
  sprintf(pid, "%d\n", getpid());
  write(fd, pid, strlen(pid));
  close(fd);
}



void daemonize(const char *outDir){
  pid_t pid;

  pid = fork();
  if (pid < 0)
    exit(EXIT_FAILURE);
  if (pid > 0)
    exit(EXIT_SUCCESS);


  if(setsid() < 0)
    exit(EXIT_FAILURE);


  pid = fork();
  if (pid < 0)
    exit(EXIT_FAILURE);
  if (pid > 0)
    exit(EXIT_SUCCESS);

  umask(0);

  chdir("/");

  for (int i = sysconf(_SC_OPEN_MAX); i >=0; i--)
    close(i);

  open(outDir,O_RDWR);
  dup(0);
  dup(0);

  openlog("File_watch", LOG_PID, LOG_DAEMON);

}

void sig_handler(int signo){
  if (signo == SIGTERM) {
  syslog(LOG_INFO, "Got SIGTERM, shutting down");
  unlink(PID_FILE);
  closelog();
  exit(EXIT_SUCCESS);
  }
}

















