#include <string.h>
#include "ylog.h"
#include "ydefs.h"

/*
** malloc0()
** Like malloc, but fill the allocated space with zeros.
*/
void *malloc0(size_t size)
{
  void *p = NULL;

  if (size > 0 && (p = malloc(size)))
    p = memset(p, 0, size);
  if (!p)
    YLOG_ADD(YLOG_ERR, "Malloc error - size=%d", size);
  return (p);
}
