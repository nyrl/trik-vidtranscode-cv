#include <stdlib.h>
#include <sysexits.h>
#include <string.h>

#include <xdc/std.h>
#include <ti/xdais/xdas.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <ti/sdo/ce/CERuntime.h>


int main()
{
  CERuntime_init();

  Memory_AllocParams allocParams;
  const size_t allocSize = 0x1000; // 4096

  memset(&allocParams, 0, sizeof(allocParams));
  allocParams.type = Memory_CONTIGPOOL;
  allocParams.flags = Memory_CACHED;
  allocParams.align = allocSize; // Just to be cool
  allocParams.seg = 0;

  printf("Allocating %zu bytes via CMEM\n", allocSize);
  void* allocBuf = Memory_alloc(allocSize, &allocParams);
  if (allocBuf == NULL)
    exit(EX_OSERR);

  printf("Memory_alloc() returned %p, verifying memset for %zu bytes\n", allocBuf, allocSize);
  memset(allocBuf, -1, allocSize);

  printf("memset(%zu) verified, releasing memory\n", allocSize);
  Memory_free(allocBuf, allocSize, &allocParams);

  printf("Done\n");
  return EX_OK;
}

