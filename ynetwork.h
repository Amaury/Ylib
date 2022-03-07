#ifndef __YNETWORK_H__
#define __YNETWORK_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

/*!
 * @typedef	yproto_t
 *		Network protocol.
 * @constant	YTCP	TCP/IP protocol.
 * @constant	YUDP	UDP/IP protocol.
 */
typedef enum yproto_s
{
  YTCP = 0,
  YUDP
} yproto_t;

/*!
 * @function	ydaemon
 *		Daemonize the program.
 * @param	serv_name	Service name.
 * @param	serv_proto	Service protocol.
 * @param	env_serv_port	Name of the service port environment variable.
 * @param	port		Pointer to integer that will get the port.
 */
void ydaemon(char *serv_name, yproto_t serv_proto,
	     char *env_serv_port, int *port);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YNETWORK_H__ */
