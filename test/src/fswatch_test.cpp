#include <stdio.h>
#include <stdlib.h>
#include <libfswatch/c/libfswatch.h>
#include <thread>
#include <unistd.h>

/**
 * $ ${CC} -I /usr/local/include -o "fswatch_test" fswatch_test.c /usr/local/lib/libfswatch.dylib
 */

/**
 * The following function implements the callback functionality for testing
 * eventnumber send from the libdawatch library. See FSW_CEVENT_CALLBACK for
 * details.
 *
 * @param events
 * @param event_num
 * @param data
 */
void my_callback(fsw_cevent const *const events,
                 const unsigned int event_num,
                 void *data)
{
  printf("my_callback: %d\n", event_num);
}

void *start_monitor(void *param)
{
  FSW_HANDLE *handle = (FSW_HANDLE *) param;

  if (FSW_OK != fsw_start_monitor(*handle))
  {
    fprintf(stderr, "Error creating thread\n");
  }
  else
  {
    printf("Monitor stopped \n");
  }

  return NULL;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("usage: %s [path]\n", argv[0]);
    return 1;
  }

  if (FSW_OK != fsw_init_library())
  {
    fsw_last_error();
    printf("libfswatch cannot be initialised!\n");
    return 1;
  }

  FSW_HANDLE handle = fsw_init_session(fsevents_monitor_type);

  if (handle == NULL)
  {
    fsw_last_error();
    printf("Invalid fswatch handle: %p\n", handle);
    return 2;
  }

  for (int i = 1; i < argc; i++)
  {
    if (FSW_OK != fsw_add_path(handle, argv[i]))
    {
      fsw_last_error();
    }
  }

  if (FSW_OK != fsw_set_callback(handle, my_callback, NULL))
  {
    fsw_last_error();
  }

  fsw_set_allow_overflow(handle, 0);

  std::thread start_thread(start_monitor, &handle);

  std::this_thread::sleep_for(std::chrono::seconds(5));

  if (FSW_OK != fsw_stop_monitor(handle))
  {
    fprintf(stderr, "Error stopping monitor \n");
    return 1;
  }

  std::this_thread::sleep_for(std::chrono::seconds(3));

  if (FSW_OK != fsw_destroy_session(handle))
  {
    fprintf(stderr, "Error destroying session\n");
    return 1;
  }

  // Wait for the monitor thread to finish
  start_thread.join();

  return 0;
}
