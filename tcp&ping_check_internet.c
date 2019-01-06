#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

#include <netdb.h>
#include <sys/syslog.h>


#include "pi_common.h"
#include "sys_module.h"
#include "sys_option.h"
#include "wifi.h"

#define nvram_safe_get(name) (nvram_get(name) ? : "")


extern int recv_pack_num;
extern int icmp_main(int argc, char **argv);
extern RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg);
extern int get_wan_connstatus(void);
extern P_WIFI_INFO_STRUCT gpi_wifi_get_info();
extern char *wifi_mode_to_str(P_WIFI_INFO_STRUCT wifi_info);

//获取IP地址
static PI8 *gethostbyname_get_ipaddr(char *str)
{
	struct hostent *hptr;
	static PI8 dst[32] = {0};
	if((hptr = gethostbyname(str)) == NULL)
	{
		return NULL;
	}
	else
	{
		inet_ntop(hptr->h_addrtype,hptr->h_addr,dst,sizeof(dst));
		return dst;
	}
}

/*
 Function name:tcp_check_internet
 Describe:use the tcp check internet ,for itb 
 Author: yinjun
 Date: 2018.10.8
 */
 
int tcp_check_internet()
{
	int connect_success_count = 0;
	char temp[128] = {0};
	char *ipaddr_buf[4] = {0};
	char *domain_buf[4] = {"google.com.br","customize.intelbras.com.br","intelbras.com.br","bing.com"};
	int i = 0;

	for(; i < 4 && (connect_success_count < 1); i++)
	{
		/*创建套接字*/
		int socket_fd = socket(PF_INET,SOCK_STREAM,0);
		if(socket_fd == -1)
		{
		  printf("Failed to create socket\n");
		  exit(-1);
		}
		
		/*初始化服务端地址结构*/
		struct sockaddr_in ser_addr;
		bzero(&ser_addr,sizeof(ser_addr));
		ser_addr.sin_family = AF_INET;
		ser_addr.sin_port = htons(80);

		/*解析域名*/
		if((ipaddr_buf[i] = gethostbyname_get_ipaddr(domain_buf[i])) != NULL)
		{
			ser_addr.sin_addr.s_addr = inet_addr(ipaddr_buf[i]);
			
			/*发送连接请求*/
			if(connect(socket_fd,(struct sockaddr*)&ser_addr,sizeof(ser_addr)) == -1)
			{	
				printf("TCP connect %s fail\n",domain_buf[i]);
				snprintf(temp,sizeof(temp),"TCP connect %s fail",domain_buf[i]);
				syslog(LOG_INFO,temp);
				
			}
			else
			{
		 		connect_success_count++;
				printf("TCP connect %s success\n",domain_buf[i]);
				snprintf(temp,sizeof(temp),"TCP connect %s success",domain_buf[i]);
				syslog(LOG_INFO,temp);
			}		
		}
		else
		{
			printf("The network is unable to connect\n");
			snprintf(temp,sizeof(temp),"gethostbyname %s parsing failure",domain_buf[i]);
			syslog(LOG_INFO,temp);
		}
		
		memset(temp,0x0,sizeof(temp));
		close(socket_fd);
		sleep(1);
	}

	return connect_success_count ? 1 : 0 ;

}

/*
yinjun:检测接入方式类型，如果是dhcp或pppoe则返回1，否则返回0 
*/
int check_wan_proto_type()
{
	int status = 0;
	char wan_proto[16] = {0};
	strcpy__(wan_proto,nvram_safe_get("wan0_proto"));

	if(0 == strcmp(wan_proto,"dhcp") || 0 == strcmp(wan_proto,"pppoe") )
	{
		status = 1;
		return status;
	}
	else
	{
		printf("wan connect type not dhcp or pppoe\n");
		return status;
	}

}

void ping_bing_check_main(cyg_addrword_t data)
{
	int timeout = 60;
	int error_time = 3;
	/*
	char *argv2[] = {"bing.com", NULL};
	char *argv3[] = {"google.com.br", NULL};
	char *argv4[] = {"customize.intelbras.com.br", NULL};
	char *argv5[] = {"intelbras.com.br", NULL};
	char *argv6[] = {"8.8.8.8", NULL};
	*/
	char gateway[32] = {0};	
	char msg_param[PI_BUFLEN_256] = {0};
	char work_mode[32] = {0};
	WIFI_INFO_STRUCT wireless_info;
		
	int status = 0;
	int wan_proto_type = 0;
	
	while(1)
	{
		//若wan口状态是connected，且wifi_mode不是wisp和apclient模式,则要开始ping
		//判断wan口模式状态，0为disconnect，1为connecting，2为connected
		//判断wan口接入方式是否为DHCP或PPPOE
		status = get_wan_connstatus();
		wan_proto_type = check_wan_proto_type();
		
		if(2 == status && 1 == wan_proto_type)
		{
			//判断wifi_mode是不是桥接模式
			memset(&wireless_info, 0x0, sizeof(WIFI_INFO_STRUCT));
			memcpy(&wireless_info, gpi_wifi_get_info(), sizeof(WIFI_INFO_STRUCT));
			strcpy(work_mode, wifi_mode_to_str(&wireless_info));
			if(0 == strcmp(work_mode, "disabled") || 0 == strcmp(work_mode, "ap"))
			{
				cyg_thread_delay(timeout*100);//每60s
				if(1 == tcp_check_internet())
				{
					printf("tcp_check_internet is success\n");
				}
				else
				{
					printf("tcp connect fail,ping check start\n");
					strcpy__(gateway,nvram_safe_get("wan0_gateway"));

					if(0 != strcmp(gateway,"") && 0 != strcmp(gateway,"0.0.0.0") && strlen(gateway) >= 7)
					{	
						char *argv2[] = {gateway,NULL};
						icmp_main(1, argv2);
						
						if(recv_pack_num == 0)
						{	
							syslog(LOG_INFO,"ping gateway failed");
							sprintf(msg_param, "op=%d", OP_RESTART);
							msg_send(MODULE_RC, RC_WAN_MODULE, msg_param);
						}
						else
						{
							syslog(LOG_INFO,"ping gateway success");
						}
					}
					else
					{
						syslog(LOG_INFO,"gateway get failed");
					}
									 
					
#if 0
					if(recv_pack_num == 0)
					{	
						cyg_thread_delay(100);
						icmp_main(1, argv3);
						if(recv_pack_num == 0)
						{
							cyg_thread_delay(100);
							icmp_main(1, argv4);
							if(recv_pack_num == 0)
							{
								cyg_thread_delay(100);
								icmp_main(1, argv5);
								if(recv_pack_num == 0)
								{
									cyg_thread_delay(100);
									icmp_main(1, argv6);
								}
							}
						}
						
						//最后没ping通，重新获取地址
						
						if(recv_pack_num == 0)
						{	
							syslog(LOG_INFO,"ping failed");
							sprintf(msg_param, "op=%d", OP_RESTART);
							msg_send(MODULE_RC, RC_WAN_MODULE, msg_param);
						}
					}
#endif
				}
				
			}
		}
		cyg_thread_delay(100);//延迟调其他线程
	}
}


char ping_check_stack[16*1024];
cyg_handle_t ping_check_handle;
cyg_thread ping_check_obj;

void ping_create_check_daemon(void)
{
	/* Create the thread */
	cyg_thread_create((cyg_addrword_t)17,
	    			    (cyg_thread_entry_t  *)   ping_bing_check_main,
	        			(cyg_addrword_t)   0,
	         			"ping_bing_check",
	          		    (void *)&ping_check_stack[0],
	           		    sizeof(ping_check_stack),
	         	  		&ping_check_handle,
	        	   		&ping_check_obj);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(ping_check_handle);
}


