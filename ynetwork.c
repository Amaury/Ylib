#include <limits.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include "ynetwork.h"
#include "ylog.h"

/*
** ydaemon()
** Daemonize the program.
*/
void ydaemon(char *serv_name, yproto_t serv_proto,
	     char *env_serv_port, int *port)
{
  int fd, nbr_fd;
  struct servent *serv;
  char *serv_env;
  char *proto_code[] = {"tcp", "udp"};

  YLOG_ADD(YLOG_DEBUG, "Entering");

  /* detach process */
  if (fork() != 0)
    exit(0);
  setsid();

  /* close all file descriptors */
  nbr_fd = getdtablesize();
  for (fd = 0; nbr_fd > 0 && fd < nbr_fd; ++fd)
    close(fd);
  chdir("/tmp");

  /* set umask */
  umask(0);

  /* get port number */
  if (port)
    {
      if (env_serv_port && (serv_env = getenv(env_serv_port)))
	*port = atoi(serv_env);
      else if (serv_name &&
	       (serv = getservbyname(serv_name, proto_code[serv_proto])))
	*port = serv->s_port;
      else
	*port = -1;
    }
  YLOG_ADD(YLOG_DEBUG, "Exiting");
}
