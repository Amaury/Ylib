/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	ylock.h
 * @abstract	All definitions for file locking.
 * @discussion	This file contains all functions needed to create lock on
 *		files. To create a lock file, you give the name of the file
 *		to lock, and a new file will be created, with the same name
 *		but with a '.lck' extension. This file contains the process
 *		ID of the program which creates the lock file.
 * @version	1.0 Oct 05 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YLOCK_H__
#define __YLOCK_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "ydefs.h"
#include "yerror.h"
#include "ystr.h"
#include "ylog.h"

/*! @define YLOCK_FILE_SUFFIX Default suffix of lock files. */
#define YLOCK_FILE_SUFFIX	".lck"
/*! @define YLOCK_DEFAULT_DELAY Default delay before lock retry. */
#define YLOCK_DEFAULT_DELAY	2

/*!
 * @function	ylock
 *		Lock a file, using the default retry delay.
 * @param	filename	Name of the file to lock.
 * @return	YENOERR if ok, YEAGAIN if the file was already locked.
 */
yerr_t ylock(const char *filename);
/*!
 * @function	ylock_delay
 *		Lock a file, using a given retry delay.
 * @param	filename	Name of the file to lock.
 * @param	delay		Retry delay in seconds.
 * @return	YENOERR if ok, YEAGAIN if the file was already locked.
 */
yerr_t ylock_delay(const char *filename, int delay);
/*!
 * @function	yunlock
 *		Unlock a file.
 * @param	filename	Name of the locked file.
 * @return	YENOERR if OK, YENOENT if the lock file doesn't exist,
 *		YEACCES if the lock file dan't be removed.
 */
yerr_t yunlock(const char *filename);

#endif /* __YLOCK_H__ */
