
#ifndef __GPSOCKDEF_H__
#define __GPSOCKDEF_H__

/*****************************************************************************/
typedef enum{
	GNS_MDM_INIT,
	GNS_MDM_DIAL,
	GNS_MDM_HANGUP,
	GNS_TIME_OUT,	/*lcp or ipcp time out for an ack long enough */
	GNS_LCP_UP,
	GNS_IP_UP,
	GNS_USR_SEND,
	GNS_PPP_QUIT,
	GNS_ERR_CHAP,
	GNS_ERR_PAP
}GP_NET_STATE;

/***************************************************************************/
struct tagGP_COMM_OPT{
	int baudrate;
	void (*uart_reset)(int ch);
	void (*uart_open)(int ch, int baudrate, int en_int);
	void (*uart_sendc)(int ch, unsigned char data);
	int (*uart_getc)(int ch, unsigned char * data);
	int (*uart_sendready)(int channel);
	unsigned int (*get_nettick)(void);
};
typedef struct tagGP_COMM_OPT GP_COMM_OPT;

struct tagGP_COMM_MEM{
	void* (*malloc)(unsigned int nbyte);
	void (*free)(void * pt);
};
typedef struct tagGP_COMM_MEM GP_COMM_MEM; 

/*****************************************************************************/
#define MAX_PARAM_STRING	128

struct tagGP_INET_OPT{
	char gp_phone_num[MAX_PARAM_STRING];
	char gp_userid[MAX_PARAM_STRING];
	char gp_pwd[MAX_PARAM_STRING];
	char gp_fhost[MAX_PARAM_STRING];
	int gp_fport;
	char gp_modem_init_cmd[MAX_PARAM_STRING];
	char gp_modem_sub_cmd[MAX_PARAM_STRING];
	int gp_ppp_tmo;
	int gp_line_tmo;
	void (*deliver_state)(GP_NET_STATE msg);
	char gp_dns_addr[MAX_PARAM_STRING];
#ifdef GPNET_DEBUG
	void (*deliver_msg)(const char *p_str);
	void (*copy_tx)(unsigned char data);
	void (*copy_rx)(unsigned char data);
#endif	
};
typedef struct tagGP_INET_OPT GP_INET_OPT;

/****************************************************************************/
extern GP_COMM_OPT gp_comm_opt;
extern GP_COMM_MEM gp_comm_mem;
extern GP_INET_OPT gp_inet_opt;

#endif /*__GPSOCKDEF_H__*/
