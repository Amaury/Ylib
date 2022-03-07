#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SERVICE		"the_service"
#define PROTOCOL	"tcp"
#define PORT		11137
#define	SERVICE_PORT	"THE_SERVICE_PORT"

int port_num;

int mon_application(int argc, char **argv);  

int main(int argc, char **argv)
{
  int fd;
  struct servent *serv;
  char *serv_env;

  /* detach process */
  if (fork() != 0)
    exit(0);
  setsid();
  /* close all file descriptors */
  for (fd = 0; fd < OPEN_MAX; ++fd)
    close(fd);
  chdir("/");
  umask(0);

  /* get port number */
  if (ac == 2)
    port_num = atoi(av[1]);
  else if ((serv_env = getenv(SERVICE_PORT)))
    port_num = atoi(serv_env);
  else if ((serv = getservbyname(SERVICE, PROTOCOL)))
    port_num = serv->s_port;
  else
    port_num = PORT;

  return (mon_application(argc, argv));
}
