#include "memory.h"
#include "../sketchybar.h"

int main (int argc, char** argv) {
  float update_freq;
  if (argc < 3 || (sscanf(argv[2], "%f", &update_freq) != 1)) {
    printf("Usage: %s \"<event-name>\" \"<event_freq>\"\n", argv[0]);
    exit(1);
  }

  alarm(0);
  struct memory memory;
  memory_init(&memory);

  // Setup the event in sketchybar
  char event_message[512];
  snprintf(event_message, 512, "--add event '%s'", argv[1]);
  sketchybar(event_message);

  char trigger_message[1024];
  for (;;) {
    // Acquire new info
    memory_update(&memory);

    // Convert bytes to MB for easier display
    uint64_t total_mb = memory.total_memory / (1024 * 1024);
    uint64_t used_mb = memory.used_memory / (1024 * 1024);
    uint64_t free_mb = memory.free_memory / (1024 * 1024);
    uint64_t cached_mb = memory.cached_memory / (1024 * 1024);
    uint64_t wired_mb = memory.wired_memory / (1024 * 1024);
    uint64_t compressed_mb = memory.compressed_memory / (1024 * 1024);

    // Prepare the event message
    snprintf(trigger_message,
             1024,
             "--trigger '%s' total_memory='%llu' used_memory='%llu' free_memory='%llu' cached_memory='%llu' wired_memory='%llu' compressed_memory='%llu' used_percentage='%d' memory_pressure='%d'",
             argv[1],
             total_mb,
             used_mb,
             free_mb,
             cached_mb,
             wired_mb,
             compressed_mb,
             memory.used_percentage,
             memory.memory_pressure);

    // Trigger the event
    sketchybar(trigger_message);

    // Wait
    usleep(update_freq * 1000000);
  }
  return 0;
}
