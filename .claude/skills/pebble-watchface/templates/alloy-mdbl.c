#include <pebble.h>

int main(void) {
  Window *window = window_create();
  window_stack_push(window, true);

  moddable_createMachine(NULL);

  window_destroy(window);
}
