#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

struct memory {
  host_t host;
  mach_msg_type_number_t count;
  vm_statistics64_data_t vm_stats;
  vm_size_t page_size;
  
  uint64_t total_memory;
  uint64_t used_memory;
  uint64_t free_memory;
  uint64_t cached_memory;
  uint64_t wired_memory;
  uint64_t compressed_memory;
  
  int memory_pressure;
  int used_percentage;
};

static inline void memory_init(struct memory* mem) {
  mem->host = mach_host_self();
  mem->count = HOST_VM_INFO64_COUNT;
  
  // Get the page size
  host_page_size(mem->host, &mem->page_size);
  
  // Get total physical memory
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  size_t length = sizeof(uint64_t);
  sysctl(mib, 2, &mem->total_memory, &length, NULL, 0);
}

static inline void memory_update(struct memory* mem) {
  kern_return_t error = host_statistics64(mem->host,
                                          HOST_VM_INFO64,
                                          (host_info64_t)&mem->vm_stats,
                                          &mem->count);

  if (error != KERN_SUCCESS) {
    printf("Error: Could not read memory host statistics.\n");
    return;
  }

  // Calculate memory usage in bytes
  uint64_t free_pages = mem->vm_stats.free_count;
  uint64_t active_pages = mem->vm_stats.active_count;
  uint64_t inactive_pages = mem->vm_stats.inactive_count;
  uint64_t wired_pages = mem->vm_stats.wire_count;
  uint64_t compressed_pages = mem->vm_stats.compressor_page_count;
  uint64_t purgeable_pages = mem->vm_stats.purgeable_count;
  uint64_t speculative_pages = mem->vm_stats.speculative_count;

  mem->free_memory = free_pages * mem->page_size;
  mem->wired_memory = wired_pages * mem->page_size;
  mem->compressed_memory = compressed_pages * mem->page_size;
  
  // Cache includes purgeable and speculative memory
  mem->cached_memory = (purgeable_pages + speculative_pages) * mem->page_size;
  
  // Used memory is active + inactive + wired + compressed
  mem->used_memory = (active_pages + inactive_pages + wired_pages + compressed_pages) * mem->page_size;
  
  // Calculate memory pressure (0-100)
  mem->used_percentage = (int)((double)mem->used_memory / (double)mem->total_memory * 100.0);
  
  // Simple memory pressure calculation based on usage percentage
  if (mem->used_percentage < 60) {
    mem->memory_pressure = 0; // Low pressure
  } else if (mem->used_percentage < 80) {
    mem->memory_pressure = 1; // Medium pressure
  } else {
    mem->memory_pressure = 2; // High pressure
  }
}
