/*
void run_comm(const char * comm, FILE* location){
  pid_t pid = fork();
  FILE* output = freopen(location);
  if (output == NULL){
    perror("No or unavailible output file");
    exit(EXIT_FAILURE);
  }
  //use sh to execute our command 
  execlp("sh", "sh", "-c", comm, NULL);
}*/
//
                                                              // using (*fds)[2] which is pointer to array of two pollfd
                                                              // if i used *fds[2] it would be array to two pointers  
void init_inotify(int* argc, char* argv[], int *fd, int** wd, nfds_t* nfds, struct pollfd (*fds)[2]){
  int i;

  //create file descriptor and ensure successful creation
  *fd = inotify_init1(IN_NONBLOCK);
  if (*fd == -1) {perror("Init error; inotify_inti1"); exit(EXIT_FAILURE);}

  //add directories into the watch list
  //we are watching for file close and for file modify
  for (i = 1; i < *argc; i++) {
    //IN_MODIFY doesnt seem to really work for VIM
    (*wd)[i] = inotify_add_watch(*fd, argv[i], IN_CLOSE | IN_MOVE);
    //ensure add success
    if((*wd)[i] == -1) {
      perror("unable to add one or more files to watch");
      exit(EXIT_FAILURE);
    }
  }

  //basically just a long int for some safety
  *nfds = 2;

  (*fds)[0].fd = STDIN_FILENO;
  (*fds)[0].events = POLLIN;

  (*fds)[1].fd = *fd;
  (*fds)[1].events = POLLIN;
}

