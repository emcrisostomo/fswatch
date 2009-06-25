#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <CoreServices/CoreServices.h> 

/* fswatch.c
 * 
 * usage: ./fswatch /some/directory /some/command
 * /some/command is executed with /some/directory as an arg
 *
 * compile me with something like: gcc fswatch.c -framework CoreServices -o fswatch
 *
 * adapted from the FSEvents api PDF
*/

char *to_watch;
char *to_run;

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
    char *envp[1] = { 0 };
    char *args[8] = {
      to_run,
      to_watch,
      0
    };
    if(execve(args[0], args, envp) < 0) {
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

  if(argc != 3) {
    fprintf(stderr, "You must specify a directory to watch and a command to execute on change\n");
    exit(1);
  }

  to_watch = argv[1];
  to_run = argv[2];

  CFStringRef mypath = CFStringCreateWithCString(NULL, to_watch, kCFStringEncodingUTF8); 
  CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL); 
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
