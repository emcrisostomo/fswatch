#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <CoreServices/CoreServices.h> 

/* fswatch.c
 * 
 * usage: ./fswatch /some/directory[:/some/otherdirectory:...] "some command" 
 * "some command" is eval'd by bash when /some/directory generates any file events
 *
 * compile me with something like: gcc fswatch.c -framework CoreServices -o fswatch
 *
 * adapted from the FSEvents api PDF
*/

extern char **environ;
//the command to run
char *bash_command[4] = {
    "/bin/bash",
    "-c",
    0, // updated later
    0
};

//fork a process when there's any change in watch file
void callback( 
    ConstFSEventStreamRef streamRef, 
    void *clientCallBackInfo, 
    size_t numEvents, 
    void *eventPaths, 
    const FSEventStreamEventFlags eventFlags[], 
    const FSEventStreamEventId eventIds[]) 
{ 
  pid_t pid;
  int   status;

  /*printf("Callback called\n"); */

  if((pid = fork()) < 0) {
    fprintf(stderr, "error: couldn't fork \n");
    exit(1);
  } else if (pid == 0) {
    if(execve(bash_command[0], bash_command, environ) < 0) {
      fprintf(stderr, "error: error executing\n");
      exit(1);
    }
  } else {
    while(wait(&status) != pid)
      ;
  }
}

//set up fsevents and callback
int main(int argc, char **argv) {

  if(argc < 3) {
    fprintf(stderr, "usage: %s directory command [argument ...]\n", argv[0]);
    exit(1);
  }

  int n_args = argc - 2;

  // find total length of argument to bash -c, including spaces
  int n_bash_arg_chars = 0;
  for(int i=2; i<argc; ++i) {
    n_bash_arg_chars += strlen(argv[i]) + 1;
  }

  // build the space-separated string argument
  char bash_arg[n_bash_arg_chars];
  int i_chars = 0;
  for(int i=2; i<argc; ++i) {
    memcpy(&bash_arg[i_chars], argv[i], strlen(argv[i]) * sizeof(char));
    i_chars += strlen(argv[i]);
    bash_arg[i_chars++] = ' ';
  }
  bash_arg[i_chars - 1] = 0;

  // update the global bash command to be run
  bash_command[2] = bash_arg;

  CFStringRef mypath = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8); 
  CFArrayRef pathsToWatch = CFStringCreateArrayBySeparatingStrings (NULL, mypath, CFSTR(":"));

  void *callbackInfo = NULL; 
  FSEventStreamRef stream; 
  CFAbsoluteTime latency = 1.0;

  stream = FSEventStreamCreate(NULL,
    &callback,
    callbackInfo,
    pathsToWatch,
    kFSEventStreamEventIdSinceNow,
    latency,
    kFSEventStreamCreateFlagNone
  ); 

  FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode); 
  FSEventStreamStart(stream);
  CFRunLoopRun();

}
