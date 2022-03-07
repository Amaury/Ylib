#include <string.h>
#include "ydefs.h"
#include "yvalue.h"

/*
** yvalue_get()
** Read a value from a character string.
*/
yvalue_t yvalue_read(const char *str, yunit_t default_unit)
{
  yvalue_t res = {YUNIT_PT, 0.0};
  const char *pt;

  if (!str)
    return (res);
  for (pt = str + strlen(str);
       pt > str && IS_SPACE(*(pt - 1));
       --pt)
    ;
  if (!strcasecmp(pt - strlen("mm"), "mm") ||
      !strcasecmp(pt - strlen("millimeter"), "millimeter"))
    res.unit = YUNIT_MM;
  else if (!strcasecmp(pt - strlen("cm"), "cm") ||
	   !strcasecmp(pt - strlen("centimeter"), "centimeter"))
    res.unit = YUNIT_CM;
  else if (!strcasecmp(pt - strlen("in"), "in") ||
	   !strcasecmp(pt - strlen("inch"), "inch"))
    res.unit = YUNIT_IN;
  else if (!strcasecmp(pt - strlen("pt"), "pt") ||
	   !strcasecmp(pt - strlen("point"), "point"))
    res.unit = YUNIT_PT;
  else
    res.unit = default_unit;
  res.value = atof(str);
  return (res);
}

/*
** yvalue_get()
** Return a value in the desired unit.
*/
float yvalue_get(const yvalue_t value, yunit_t unit)
{
  float pt;

  if (value.unit == unit)
    return (value.value);
  switch (value.unit)
    {
    case YUNIT_MM:
      pt = value.value / 0.3528;
      break ;
    case YUNIT_CM:
      pt = value.value / 0.03528;
      break ;
    case YUNIT_IN:
      pt = value.value * 72;
      break;
    case YUNIT_PT:
    default:
      pt = value.value;
    }
  switch (unit)
    {
    case YUNIT_MM:
      return (pt * 0.3528);
    case YUNIT_CM:
      return (pt * 0.03528);
    case YUNIT_IN:
      return (pt / 72);
    case YUNIT_PT:
    default:
      return (pt);
    }
}
