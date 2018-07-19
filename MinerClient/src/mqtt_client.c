/**
 * @file mqtt_client.c
 * create on 2018-05-21 by wjun
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <mqtt.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <linux/sockios.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <assert.h>
#include "jansson.h"
#include "curl.h"
#include "mqtt_client.h"

volatile int quit = 0;
volatile int pquit = 0;
pthread_t client_daemon;
static bw_miner_sysinfo minerSysInfo = {0},preminerSysInfo={0};
static bw_miner_runinginfo minerRuningInfo = {0},preminerRuningInfo={0};
static bw_miner_cfginfo miner_cfg={0};

void publish_callback(void** unused, struct mqtt_response_publish *published);
void* client_refresher(void* client);
void exit_mqtt_client(int status, int sockfd, pthread_t *client_daemon);

void signal_handler(int sig)
{
	quit = 1;
	pquit = 1;
}

int bw_semCreate(bw_SemHndl *hndl, int maxCount, int initVal)
{
  pthread_mutexattr_t mutex_attr;
  pthread_condattr_t cond_attr;
  int status=0;

  status |= pthread_mutexattr_init(&mutex_attr);
  status |= pthread_condattr_init(&cond_attr);
  status |= pthread_mutex_init(&hndl->lock, &mutex_attr);
  status |= pthread_cond_init(&hndl->cond, &cond_attr);

  hndl->count = initVal;
  hndl->maxCount = maxCount;
  if(hndl->maxCount==0)
    hndl->maxCount=1;

  if(hndl->count>hndl->maxCount)
    hndl->count = hndl->maxCount;

  if(status!=0)
    bwclinetLog(LOG_ERR,"OSA_semCreate() = %d", status);

  pthread_condattr_destroy(&cond_attr);
  pthread_mutexattr_destroy(&mutex_attr);
  return status;
}

int bw_semWait(bw_SemHndl *hndl, int timeout)
{
  int status = -1;

  pthread_mutex_lock(&hndl->lock);
  struct timeval now;
  struct timespec outtime;

  while(1) {
    if(hndl->count > 0) {
      hndl->count--;
      status = 0;
      break;
    } else {
      if(timeout==OSA_TIMEOUT_NONE)
        break;
		if (timeout != OSA_TIMEOUT_NONE && timeout != OSA_TIMEOUT_FOREVER){
			gettimeofday(&now, NULL);
			outtime.tv_sec = now.tv_sec + timeout;
			outtime.tv_nsec = now.tv_usec * 1000;
			pthread_cond_timedwait(&hndl->cond, &hndl->lock, &outtime);
			status = 0;
			break;
		}else{
			pthread_cond_wait(&hndl->cond, &hndl->lock);
		}
    }
  }
  pthread_mutex_unlock(&hndl->lock);
  return status;
}

int bw_semSignal(bw_SemHndl *hndl)
{
  int status = 0;
  pthread_mutex_lock(&hndl->lock);
  if(hndl->count<hndl->maxCount) {
    //hndl->count++;
    status |= pthread_cond_signal(&hndl->cond);
  }
  pthread_mutex_unlock(&hndl->lock);
  return status;
}

int bw_semDelete(bw_SemHndl *hndl)
{
  pthread_cond_destroy(&hndl->cond);
  pthread_mutex_destroy(&hndl->lock);
  return 0;
}

char* cmd_system(const char* command)
{
    char* result = "";
    FILE *fpRead;
    fpRead = popen(command, "r");
    char buf[1024];
    memset(buf,'\0',sizeof(buf));
    while(fgets(buf,1024-1,fpRead)!=NULL)
    {result = buf;}
    if(fpRead!=NULL)
        pclose(fpRead);
    return result;
}

int get_local_ip_address(char *name,char *net_ip)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) 
	{
		ret = -1;
	}
	else
		strcpy(net_ip,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}

int get_local_mac_address(char *name, char *net_mac)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;

	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;

	ret = ioctl(fd, SIOCGIFHWADDR,(char *)&ifr,sizeof(ifr));
    if(ret == 0)
	{
        memcpy(net_mac,ifr.ifr_hwaddr.sa_data,6);
	}

	close(fd);

	return	ret;
}
int get_local_gateway_address(char *name,char *gateway_addr)
{
	char buff[256];
	char ifname[32] = {0};
	int  nl = 0 ;
	struct in_addr gw;
	int flgs, ref, use, metric;
	unsigned  long d,g,m;

	FILE	*fp;
	
	if((fp = fopen("/proc/net/route", "r")) == NULL)
		return -1;	
		
	nl = 0 ;
	while( fgets(buff, sizeof(buff), fp) != NULL ) 
	{
		if(nl) 
		{
			//int ifl = 0;
			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				   ifname,&d, &g, &flgs, &ref, &use, &metric, &m)!=8) 
			{
				//continue;
				fclose(fp);
				return	-2;
			}
			//ifl = 0;        /* parse flags */
			if(flgs&RTF_UP && (strcmp(name,ifname)== 0)) 
			{			
				gw.s_addr   = g;
					
				if(d==0)
				{
					strcpy(gateway_addr,inet_ntoa(gw));
					fclose(fp);
					return 0;
				}				
			}
		}
		nl++;
	}	
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}
	return	-1;
}
int get_local_mask_address(char *name,char *net_mask)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;	
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) 
	{
		ret = -1;
	}
	
	strcpy(net_mask,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}
int get_cfg_dns_address(char *cfgFile, unsigned int *pDns, unsigned int *pDns2)
{
	char szStr[128] = {0};
	char ss[128]  = {0};
	FILE *pFile;
	int  nArray[4]   = {0};
	unsigned int mIP = 0;
	int i = 0;
		
	if((pFile = fopen(cfgFile, "r+b")) == NULL)
		return -1;
	
	while(fgets(szStr, 128, pFile) != NULL)
	{
		if(strstr(szStr,"nameserver") == NULL)
			continue ;
		
		if(sscanf(szStr, "%s %d.%d.%d.%d", ss, &(nArray[0]), &(nArray[1]),
			&(nArray[2]), &(nArray[3])) == 5)
		{
			mIP = (nArray[0] << 24) | (nArray[1] << 16) |
				  (nArray[2] << 8)  | (nArray[3] << 0);
			if(i == 0 && pDns != NULL)
			{
				*pDns = mIP;
			}
			else if (pDns2 != NULL)
			{
				*pDns2 = mIP;
			}
			i++;				
		}
		memset(szStr, 0, 128);
	}
	fclose(pFile);
	return 0;
}

int Devs_get_when(const char *value,int index){return 0;}
int Devs_get_Code(const char *value,int index){return 0;}
int Devs_get_Msg(const char *value,int index){return 0;}
int Devs_get_Description(const char *value,int index){return 0;}
int Devs_get_ASC(const char *value,int index){return 0;}
int Devs_get_Name(const char *value,int index){return 0;}
int Devs_get_ID(const char *value,int index){return 0;}
int Devs_get_Enabled(const char *value,int index){return 0;}
int Devs_get_MHS_1m(const char *value,int index){return 0;}
int Devs_get_MHS_5m(const char *value,int index){return 0;}
int Devs_get_MHS_15m(const char *value,int index){return 0;}
int Devs_get_Hardware_Errors(const char *value,int index){return 0;}
int Devs_get_Device_Elapsed(const char *value,int index){return 0;}
int Devs_get_status(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	strncpy(minerRuningInfo.status,value,sizeof(minerRuningInfo.status));
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

int Devs_get_MHS_av(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_avg_mhs = atoi(value);break;}
		case 1:{minerRuningInfo.board2_avg_mhs = atoi(value);break;}
		case 2:{minerRuningInfo.board3_avg_mhs = atoi(value);break;}
		case 3:{minerRuningInfo.board4_avg_mhs = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}
int Devs_get_MHS_5s(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_mhs = atoi(value);break;}
		case 1:{minerRuningInfo.board2_mhs = atoi(value);break;}		
		case 2:{minerRuningInfo.board3_mhs = atoi(value);break;}		
		case 3:{minerRuningInfo.board4_mhs = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}
int Devs_get_Accepted(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_accept = atoi(value);break;}
		case 1:{minerRuningInfo.board2_accept = atoi(value);break;}			
		case 2:{minerRuningInfo.board3_accept = atoi(value);break;}			
		case 3:{minerRuningInfo.board4_accept = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}
int Devs_get_Rejected(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_reject = atoi(value);break;}
		case 1:{minerRuningInfo.board2_reject = atoi(value);break;}
		case 2:{minerRuningInfo.board3_reject = atoi(value);break;}
		case 3:{minerRuningInfo.board4_reject = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}
int Devs_get_temperature(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	
	if (0 == strcmp(minerSysInfo.type,L61))
		return 0;
	
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_temperate = atoi(value);break;}
		case 1:{minerRuningInfo.board2_temperate = atoi(value);break;}
		case 2:{minerRuningInfo.board3_temperate = atoi(value);break;}
		case 3:{minerRuningInfo.board4_temperate = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}
int Devs_get_frequency(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_frequency = atoi(value);break;}
		case 1:{minerRuningInfo.board2_frequency = atoi(value);break;}
		case 2:{minerRuningInfo.board3_frequency = atoi(value);break;}
		case 3:{minerRuningInfo.board4_frequency = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

static option const Devs_options[] = {
	{ "STATUS",Devs_get_status},
	{ "When",Devs_get_when},
	{ "Code",Devs_get_Code},
	{ "Msg",Devs_get_Msg},
	{ "Description",Devs_get_Description},
	{ "ASC",Devs_get_ASC},
	{ "Name",Devs_get_Name},
	{ "ID",Devs_get_ID},
	{ "Enabled",Devs_get_Enabled},
	{ "MHS av",Devs_get_MHS_av},
	{ "MHS 5s",Devs_get_MHS_5s},
	{ "MHS 1m",Devs_get_MHS_1m},
	{ "MHS 5m",Devs_get_MHS_5m},
	{ "MHS 15m",Devs_get_MHS_15m},
	{ "Accepted",Devs_get_Accepted},
	{ "Rejected",Devs_get_Rejected},
	{ "Hardware Errors",Devs_get_Hardware_Errors},
	{ "Device Elapsed",Devs_get_Device_Elapsed},
	{ "temperature",Devs_get_temperature},
	{ "frequency",Devs_get_frequency},
	{"0",NULL}
};

int Summary_get_status(const char *value,int index){return 0;}
int Summary_get_summary(const char *value,int index){return 0;}
int Summary_get_run_time(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	minerRuningInfo.duration = atoi(value);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}
int Summary_get_enable_paas(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	if (!strcmp(value,"true")){
		minerSysInfo.enable_paas = true;}
	else
		minerSysInfo.enable_paas = false;
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}
int Summary_get_paas_ip(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	strncpy(minerSysInfo.paas_ip,value,sizeof(minerSysInfo.paas_ip));
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}
int Summary_get_paas_port(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	minerSysInfo.paas_port = atoi(value);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}
int Summary_get_paas_user(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	strncpy(minerSysInfo.paas_user,value,sizeof(minerSysInfo.paas_user));
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Summary_get_paas_password(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	strncpy(minerSysInfo.paas_pwd,value,sizeof(minerSysInfo.paas_pwd));
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Summary_get_position(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	strncpy(minerSysInfo.position,value,sizeof(minerSysInfo.position));
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Summary_get_periodic_time(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	minerSysInfo.periodic_time = atoi(value);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Summary_get_fan1_speed(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	minerRuningInfo.fan1_speed = atoi(value);
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

int Summary_get_fan2_speed(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	minerRuningInfo.fan2_speed = atoi(value);
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

int Summary_get_temperature_threshould(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	minerRuningInfo.temperature_threshould = atoi(value);
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

int Summary_get_mhs_threshould(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	minerRuningInfo.mhs_threshould = atoi(value);
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

int Summary_get_number(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	strncpy(minerSysInfo.number,value,sizeof(minerSysInfo.number));
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Summary_get_miner_type(const char *value,int index)
{
	if (!value)
		return -1;
	
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	strncpy(minerSysInfo.type,value,sizeof(minerSysInfo.type));
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

static option const Summary_options[] = {
	{ "STATUS",Summary_get_status},
	{ "SUMMARY",Summary_get_summary},
	{ "Elapsed",Summary_get_run_time},
	{ "usePaas",Summary_get_enable_paas},
	{ "paas_ip",Summary_get_paas_ip},
	{ "paas_port",Summary_get_paas_port},
	{ "paas_user",Summary_get_paas_user},
	{ "paas_password",Summary_get_paas_password},
	{ "position",Summary_get_position},
	{ "periodic_time",Summary_get_periodic_time},
	{ "fan1_speed",Summary_get_fan1_speed},
	{ "fan2_speed",Summary_get_fan2_speed},
	{ "temperature_threshould",Summary_get_temperature_threshould},
	{ "mhs_threshould",Summary_get_mhs_threshould},
	{ "number",Summary_get_number},
	{ "miner_type",Summary_get_miner_type},
	{"0",NULL}
};

int Pool_get_status(const char *value,int index){return 0;}
int Pools_get_status(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	switch(index)
	{
		case 0:{strncpy(minerSysInfo.pool1_status,value,sizeof(minerSysInfo.pool1_status));break;}
		case 1:{strncpy(minerSysInfo.pool2_status,value,sizeof(minerSysInfo.pool2_status));break;}
		case 2:{strncpy(minerSysInfo.pool3_status,value,sizeof(minerSysInfo.pool3_status));break;}
		default:break;
	}	
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Pools_get_Diff(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	switch(index)
	{
		case 0:{minerSysInfo.pool1_diff = atoi(value);break;}
		case 1:{minerSysInfo.pool2_diff = atoi(value);break;}
		case 2:{minerSysInfo.pool3_diff = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}
int Pools_get_URL(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	switch(index)
	{
		case 0:{strncpy(minerSysInfo.pool1_addr,value,sizeof(minerSysInfo.pool1_addr));break;}
		case 1:{strncpy(minerSysInfo.pool2_addr,value,sizeof(minerSysInfo.pool2_addr));break;}
		case 2:{strncpy(minerSysInfo.pool3_addr,value,sizeof(minerSysInfo.pool3_addr));break;}
		default:break;
	}
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Pools_get_User(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	switch(index)
	{
		case 0:{strncpy(minerSysInfo.pool1_miner_addr,value,sizeof(minerSysInfo.pool1_miner_addr));break;}
		case 1:{strncpy(minerSysInfo.pool2_miner_addr,value,sizeof(minerSysInfo.pool2_miner_addr));break;}
		case 2:{strncpy(minerSysInfo.pool3_miner_addr,value,sizeof(minerSysInfo.pool3_miner_addr));break;}
		default:break;
	}
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

int Pools_get_password(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	switch(index)
	{
		case 0:{strncpy(minerSysInfo.pool1_password,value,sizeof(minerSysInfo.pool1_password));break;}
		case 1:{strncpy(minerSysInfo.pool2_password,value,sizeof(minerSysInfo.pool2_password));break;}
		case 2:{strncpy(minerSysInfo.pool3_password,value,sizeof(minerSysInfo.pool3_password));break;}
		default:break;
	}
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	return 0;
}

static option const Pools_options[] = {
	{ "STATUS",Pool_get_status},
	{ "Status",Pools_get_status},
	{ "Diff",Pools_get_Diff},
	{ "URL",Pools_get_URL},
	{ "User",Pools_get_User},
	{ "Passwd",Pools_get_password},
	{"0",NULL}
};

int Boards_get_name(const char *value,int index){return 0;}
int Boards_get_plug_status(const char *value,int index){return 0;}
int Boards_get_DCDC_enable(const char *value,int index){return 0;}
int Boards_get_voltage(const char *value,int index){return 0;}
int Boards_get_temp2(const char *value,int index){return 0;}
int Boards_get_temp1(const char *value,int index)
{
	if (!value || index < 0)
		return -1;

	pthread_mutex_lock(&minerRuningInfo.runinginfo_lock);
	switch(index)
	{
		case 0:{minerRuningInfo.board1_temperate = atoi(value);break;}
		case 1:{minerRuningInfo.board2_temperate = atoi(value);break;}
		case 2:{minerRuningInfo.board3_temperate = atoi(value);break;}
		case 3:{minerRuningInfo.board4_temperate = atoi(value);break;}
		default:break;
	}
	pthread_mutex_unlock(&minerRuningInfo.runinginfo_lock);
	return 0;
}

static option const Boards_options[] = {
	{ "Name",Boards_get_name},
	{ "plug",Boards_get_plug_status},
	{ "temperature1",Boards_get_temp1},
	{ "temperature2",Boards_get_temp2},
	{ "DCDC_enable",Boards_get_DCDC_enable},
	{ "voltage",Boards_get_voltage},
	{"0",NULL}
};

int Miner_check_mac(const char *value,int index)
{
	if (!value)
		return -1;
	
	unsigned char mac[16];
	char strMac[32];
	if(get_local_mac_address("eth0", (char *)mac) < 0)
	{
		bwclinetLog(LOG_ERR,"Failed to get eth0 mac addr");
		return -1;
	}
	snprintf(strMac, sizeof(strMac), "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);	

	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	if (0 != strcmp(strMac,value)){
		miner_cfg.match = 0;
		pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
		return -1;
	}
	miner_cfg.match = 1;
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_ip_type(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.ip_type,value,sizeof(miner_cfg.ip_type));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_ip(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.ip,value,sizeof(miner_cfg.ip));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_netmask(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.netmask,value,sizeof(miner_cfg.netmask));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_gateway(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.gateway,value,sizeof(miner_cfg.gateway));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_dns(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.dns,value,sizeof(miner_cfg.dns));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool1_addr(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool1_addr,value,sizeof(miner_cfg.pool1_addr));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool1_miner_addr(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool1_miner_addr,value,sizeof(miner_cfg.pool1_miner_addr));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool1_password(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool1_password,value,sizeof(miner_cfg.pool1_password));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool2_addr(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool2_addr,value,sizeof(miner_cfg.pool2_addr));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool2_miner_addr(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool2_miner_addr,value,sizeof(miner_cfg.pool2_miner_addr));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool2_password(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool2_password,value,sizeof(miner_cfg.pool2_password));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool3_addr(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool3_addr,value,sizeof(miner_cfg.pool3_addr));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool3_miner_addr(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool3_miner_addr,value,sizeof(miner_cfg.pool3_miner_addr));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_pool3_password(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.pool3_password,value,sizeof(miner_cfg.pool3_password));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_position(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.position,value,sizeof(miner_cfg.position));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_number(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.number,value,sizeof(miner_cfg.number));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_update_url(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	strncpy(miner_cfg.update_url,value,sizeof(miner_cfg.update_url));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_mhs_threshould(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.mhs_threshould = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_temp_threshould(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.temp_threshould = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_fan1_speed(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.fan_speed[0] = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_fan2_speed(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.fan_speed[1] = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_periodic_time(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.periodic_time = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_use_frequency_all(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	if (!strcmp(value,"true")){
		miner_cfg.use_frequency_all = true;}
	else
		miner_cfg.use_frequency_all = false;
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_frequency_all(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.frequency_all = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_frequency1(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.frequency_1 = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_frequency2(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.frequency_2 = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_set_frequency3(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.frequency_3 = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_set_frequency4(const char *value,int index)
{
	if (!value)
		return -1;
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.frequency_4 = atoi(value);
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}

int Miner_get_batchset_ips(const char *value,int index)
{
	if (!value || index < 0 || index > 255)
		return -1;

	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	memcpy(miner_cfg.ipAarry[index],value,strlen(value));
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_batch_set_data(const char *value,int index){return 0;}
int Miner_batch_set_pools_name(const char *value,int index){return 0;}
int Miner_batch_set_pools_addr(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	switch(index)
	{
		case 0:{strncpy(miner_cfg.pool1_addr,value,sizeof(miner_cfg.pool1_addr));break;}
		case 1:{strncpy(miner_cfg.pool2_addr,value,sizeof(miner_cfg.pool2_addr));break;}
		case 2:{strncpy(miner_cfg.pool3_addr,value,sizeof(miner_cfg.pool3_addr));break;}
		default:break;
	}
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_batch_set_pools_minerAddr(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	switch(index)
	{
		case 0:{strncpy(miner_cfg.pool1_miner_addr,value,sizeof(miner_cfg.pool1_miner_addr));break;}
		case 1:{strncpy(miner_cfg.pool2_miner_addr,value,sizeof(miner_cfg.pool2_miner_addr));break;}
		case 2:{strncpy(miner_cfg.pool3_miner_addr,value,sizeof(miner_cfg.pool3_miner_addr));break;}
		default:break;
	}
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int Miner_batch_set_pools_password(const char *value,int index)
{
	if (!value || index < 0)
		return -1;
	
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	switch(index)
	{
		case 0:{strncpy(miner_cfg.pool1_password,value,sizeof(miner_cfg.pool1_password));break;}
		case 1:{strncpy(miner_cfg.pool2_password,value,sizeof(miner_cfg.pool2_password));break;}
		case 2:{strncpy(miner_cfg.pool3_password,value,sizeof(miner_cfg.pool3_password));break;}
		default:break;
	}
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}


static option const Miner_cfg_options[] = {
	{ "mac",Miner_check_mac},
	{ "ip_type",Miner_set_ip_type},
	{ "ip",Miner_set_ip},
	{ "netmask",Miner_set_netmask},
	{ "gateway",Miner_set_gateway},
	{ "dns",Miner_set_dns},
	{ "pool1_addr",Miner_set_pool1_addr},
	{ "pool1_miner_addr",Miner_set_pool1_miner_addr},
	{ "pool1_password",Miner_set_pool1_password},
	{ "pool2_addr",Miner_set_pool2_addr},
	{ "pool2_miner_addr",Miner_set_pool2_miner_addr},
	{ "pool2_password",Miner_set_pool2_password},
	{ "pool3_addr",Miner_set_pool3_addr},
	{ "pool3_miner_addr",Miner_set_pool3_miner_addr},
	{ "pool3_password",Miner_set_pool3_password},
	{ "position",Miner_set_position},
	{ "ips",Miner_get_batchset_ips},
	{ "data",Miner_batch_set_data},
	{ "name",Miner_batch_set_pools_name},
	{ "addr",Miner_batch_set_pools_addr},
	{ "minerAddr",Miner_batch_set_pools_minerAddr},
	{ "password",Miner_batch_set_pools_password},
	{ "number",Miner_set_number},
	{ "url",Miner_set_update_url},
	{ "mhs_threshould",Miner_set_mhs_threshould},
	{ "temperature_threshould",Miner_set_temp_threshould},
	{ "fan1_speed",Miner_set_fan1_speed},
	{ "fan2_speed",Miner_set_fan2_speed},
	{ "periodic_time",Miner_set_periodic_time},
	{ "use_frequency_all",Miner_set_use_frequency_all},
	{ "frequency",Miner_set_frequency_all},
	{ "frequency1",Miner_set_frequency1},
	{ "frequency2",Miner_set_frequency2},
	{ "frequency3",Miner_set_frequency3},
	{ "frequency4",Miner_set_frequency4},
	{ "0",NULL}
};

int bw_mqtt_miner_get_devs_info()
{
	FILE *stream = NULL;   
	char recvBuf[3*BUFSIZE] = {0}; 
	char cmdBuf[128] = {0};

	sprintf(cmdBuf, "echo -n '{\"command\":\"devs\"}' | nc localhost 4028 ");	
	stream = popen(cmdBuf, "r");   
	fread(recvBuf, sizeof(char), sizeof(recvBuf)-1, stream);   
	pclose(stream);   

	//解析json data
	json_t *objectmsg = NULL;
	json_t *json_val;
	json_error_t json_err;
	size_t i = 0,j = 0;

	size_t devs_array_size = ARRAY_SIZE(Devs_options);
	objectmsg = json_loadb(recvBuf,strlen(recvBuf), 0, &json_err);
	if (json_is_object(objectmsg))
	{
		json_val = json_object_get(objectmsg, "STATUS");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<devs_array_size;++j)
				{
					if (!Devs_options[j].name)
						break;
					value = json_object_get(obj, Devs_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					Devs_options[j].func(s,i);
					free(s);			
				}		
			}
		}else{}

		json_val = json_object_get(objectmsg, "DEVS");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<devs_array_size;++j)
				{
					if (!Devs_options[j].name)
						break;
					value = json_object_get(obj, Devs_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = (char *)malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}
					if (!s)
						continue;
					Devs_options[j].func(s,i);
					free(s);
					s = NULL;
				}		
			}
		}else{}
	}else{bwclinetLog(LOG_ERR,"%s it's not json data",__func__);}
	json_decref(objectmsg);
	return 0;	
}

int bw_mqtt_miner_get_summary_info()
{
	FILE *stream = NULL;   
	char recvBuf[3*BUFSIZE] = {0}; 
	char cmdBuf[128] = {0};

	sprintf(cmdBuf, "echo -n '{\"command\":\"summary\"}' | nc localhost 4028 ");   
	stream = popen(cmdBuf, "r");   
	fread(recvBuf, sizeof(char), sizeof(recvBuf)-1, stream);   
	pclose(stream);   

	//解析json data
	json_t *objectmsg = NULL;
	json_t *json_val;
	json_error_t json_err;
	size_t i = 0,j = 0;

	size_t summary_array_size = ARRAY_SIZE(Summary_options);
	objectmsg = json_loadb(recvBuf,strlen(recvBuf), 0, &json_err);
	if (json_is_object(objectmsg))
	{
		json_val = json_object_get(objectmsg, "STATUS");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<summary_array_size;++j)
				{
					if (!Summary_options[j].name)
						break;
					value = json_object_get(obj, Summary_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					Summary_options[j].func(s,i);
					free(s);			
				}		
			}
		}else{}

		json_val = json_object_get(objectmsg, "SUMMARY");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<summary_array_size;++j)
				{
					if (!Summary_options[j].name)
						break;
					value = json_object_get(obj, Summary_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = (char *)malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					Summary_options[j].func(s,i);
					free(s);
					s = NULL;
				}		
			}
		}else{}
	}else{bwclinetLog(LOG_ERR,"%s it's not json data",__func__);}

	json_decref(objectmsg);
	return 0;	
}
int bw_mqtt_miner_get_pools_info()
{
	FILE *stream = NULL;   
	char recvBuf[3*BUFSIZE] = {0}; 
	char cmdBuf[128] = {0};

	sprintf(cmdBuf, "echo -n '{\"command\":\"pools\"}' | nc localhost 4028 ");   
	stream = popen(cmdBuf, "r");   
	fread(recvBuf, sizeof(char), sizeof(recvBuf)-1, stream);   
	pclose(stream);   

	//解析json data
	json_t *objectmsg = NULL;
	json_t *json_val;
	json_error_t json_err;
	size_t i = 0,j = 0;

	size_t pools_array_size = ARRAY_SIZE(Pools_options);
	objectmsg = json_loadb(recvBuf,strlen(recvBuf), 0, &json_err);
	if (json_is_object(objectmsg))
	{
		json_val = json_object_get(objectmsg, "STATUS");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<pools_array_size;++j)
				{
					if (!Pools_options[j].name)
						break;
					value = json_object_get(obj, Pools_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					Pools_options[j].func(s,i);
					free(s);			
				}		
			}
		}else{}

		json_val = json_object_get(objectmsg, "POOLS");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<pools_array_size;++j)
				{
					if (!Pools_options[j].name)
						break;
					value = json_object_get(obj, Pools_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = (char *)malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					Pools_options[j].func(s,i);
					free(s);
					s = NULL;
				}		
			}
		}else{}
	}else{bwclinetLog(LOG_ERR,"%s it's not json data",__func__);}
	json_decref(objectmsg);
	return 0;	
}

int bw_mqtt_miner_get_boards_info()
{
	FILE *stream = NULL;   
	char recvBuf[BUFSIZE] = {0}; 
	char cmdBuf[128] = {0};

	sprintf(cmdBuf, "/usr/app/minerapi board");
	stream = popen(cmdBuf, "r");   
	fread(recvBuf, sizeof(char), sizeof(recvBuf)-1, stream);   
	pclose(stream);   

	//解析json data
	json_t *objectmsg = NULL;
	json_t *json_val;
	json_error_t json_err;
	size_t i = 0,j = 0;

	size_t pools_array_size = ARRAY_SIZE(Boards_options);
	objectmsg = json_loadb(recvBuf,strlen(recvBuf), 0, &json_err);
	if (json_is_object(objectmsg))
	{
		json_val = json_object_get(objectmsg, "board");
		if (json_is_array(json_val))
		{
			size_t j_array_size = json_array_size(json_val);
			for (i=0; i< j_array_size;++i)
			{
				json_t *obj,*value;
				obj = json_array_get(json_val,i);
				for (j=0;j<pools_array_size;++j)
				{
					if (!Boards_options[j].name)
						break;
					value = json_object_get(obj, Boards_options[j].name);
					if(!value)
						continue;
					char *s = NULL;
					if (json_is_string(value)){
						s = strdup(json_string_value(value));
					}
					if (json_is_number(value)){
						s = malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(value));}
					}
					if (json_is_boolean(value)){
						s = (char *)malloc(8);
						if (json_is_true(value)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(value)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					Boards_options[j].func(s,i);
					free(s);			
				}		
			}
		}else{}
	}else{bwclinetLog(LOG_ERR,"%s it's not json data",__func__);}
	json_decref(objectmsg);
	return 0;	
}

int bw_mqtt_miners_join_msg(char *jsonMsg)
{
	if (!jsonMsg){
		bwclinetLog(LOG_ERR,"%s invaild input params.",__func__);
		return -1;
	}
    json_t *objectmsg;
    char *result;
    objectmsg = json_object();
	unsigned char mac[16];
	char strMac[32];
	if(get_local_mac_address("eth0", (char *)mac) < 0)
	{
		bwclinetLog(LOG_ERR,"Failed to get eth0 mac addr.");
		return -1;
	}
	snprintf(strMac, sizeof(strMac), "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	time_t timer;
	time(&timer);
	struct tm* tm_info = localtime(&timer);
	char timebuf[26];
	strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	char szIp[16]={0};
	get_local_ip_address("eth0", szIp);
	bw_mqtt_miner_get_devs_info();
	bw_mqtt_miner_get_pools_info();
	json_object_set_new (objectmsg, "id", json_string("0"));
    json_object_set_new (objectmsg, "date", json_string(timebuf));
    json_object_set_new (objectmsg, "type", json_string(minerSysInfo.type));
    json_object_set_new (objectmsg, "ip", json_string(szIp));
    json_object_set_new (objectmsg, "mac", json_string(strMac));
    json_object_set_new (objectmsg, "status", json_string(minerRuningInfo.status));
	sprintf(minerSysInfo.ip,"%s",szIp);
	sprintf(minerSysInfo.mac,"%s",strMac);
	
	char ipType[8]={0},version[32]={0};
	char *result1 = cmd_system("cat /etc/network/interfaces | grep dhcp");
	strcmp(result1,"") == 0 ? strcpy(ipType,"static") : strcpy(ipType,"dhcp");
	char *result2 = cmd_system("cat ../firm.conf");
	strcmp(result2,"") == 0 ? strcpy(version,"undefined") : strcpy(version,result2+strlen("version"));
    json_object_set_new (objectmsg, "version", json_string(version));
	json_object_set_new (objectmsg, "ip_type", json_string(ipType));
	sprintf(minerSysInfo.version,"%s",version);
	sprintf(minerSysInfo.ip_type,"%s",ipType);
	
	char szMask[16]={0},szGatway[16]={0},szdns1[16]={0},szdns2[16]={0};
	unsigned int dns1,dns2;
	get_local_mask_address("eth0", szMask);	
	get_local_gateway_address("eth0", szGatway);
	get_cfg_dns_address("/etc/resolv.conf",&dns1,&dns2);
	sprintf(szdns1,"%d.%d.%d.%d",(dns1 >> 24) & 0xff,
								(dns1 >> 16) & 0xff,
								(dns1 >> 8) & 0xff,
								(dns1 >> 0) & 0xff);
	sprintf(szdns2,"%d.%d.%d.%d",(dns2 >> 24) & 0xff,
								(dns2 >> 16) & 0xff,
								(dns2 >> 8) & 0xff,
								(dns2 >> 0) & 0xff);	
    json_object_set_new (objectmsg, "netmask", json_string(szMask));
    json_object_set_new (objectmsg, "gateway", json_string(szGatway));
    json_object_set_new (objectmsg, "dns", json_string(szdns1));
	sprintf(minerSysInfo.netmask,"%s",szMask);
	sprintf(minerSysInfo.gateway,"%s",szGatway);
	sprintf(minerSysInfo.dns,"%s",szdns1);
	
	char str_periodic_time[8]={0};
	sprintf(str_periodic_time,"%d",minerSysInfo.periodic_time);
    json_object_set_new (objectmsg, "periodic_time", json_string(str_periodic_time));//(Web页面可编辑)
    
    json_object_set_new (objectmsg, "pool1_status", json_string(minerSysInfo.pool1_status));
    json_object_set_new (objectmsg, "pool1_addr", json_string(minerSysInfo.pool1_addr));
    json_object_set_new (objectmsg, "pool1_miner_addr", json_string(minerSysInfo.pool1_miner_addr));
    json_object_set_new (objectmsg, "pool1_password", json_string(minerSysInfo.pool1_password));
    json_object_set_new (objectmsg, "pool2_status", json_string(minerSysInfo.pool2_status));
    json_object_set_new (objectmsg, "pool2_addr", json_string(minerSysInfo.pool2_addr));
    json_object_set_new (objectmsg, "pool2_miner_addr", json_string(minerSysInfo.pool2_miner_addr));
    json_object_set_new (objectmsg, "pool2_password", json_string(minerSysInfo.pool2_password));
    json_object_set_new (objectmsg, "pool3_status", json_string(minerSysInfo.pool3_status));
    json_object_set_new (objectmsg, "pool3_addr", json_string(minerSysInfo.pool3_addr));
    json_object_set_new (objectmsg, "pool3_miner_addr", json_string(minerSysInfo.pool3_miner_addr));
    json_object_set_new (objectmsg, "pool3_password", json_string(minerSysInfo.pool3_password));
	json_object_set_new (objectmsg, "number", json_string(minerSysInfo.number));
    json_object_set_new (objectmsg, "position", json_string(minerSysInfo.position));

	char mhs[32] = {0},avg_mhs[32]={0},duration[32]={0};
	sprintf(mhs,"%d",minerRuningInfo.mhs);
	sprintf(avg_mhs,"%d",minerRuningInfo.avg_mhs);
	sprintf(duration,"%lu",minerRuningInfo.duration);
    json_object_set_new (objectmsg, "mhs", json_string(mhs));
    json_object_set_new (objectmsg, "avg_mhs", json_string(avg_mhs));
    json_object_set_new (objectmsg, "duration", json_string(duration));
    result = json_dumps(objectmsg, JSON_PRESERVE_ORDER);
	memcpy(jsonMsg,result,strlen(result));
    free(result);
	json_decref(objectmsg);
	return 0;
}

int bw_miner_sys_info_publish_msg(char *jsonMsg,int *changed_cnt)
{
	if (!jsonMsg || !changed_cnt){
		bwclinetLog(LOG_ERR,"%s invaild input params.",__func__);
		return -1;
	}
	json_t *objectmsg;
	char *result;
	objectmsg = json_object();
	unsigned char mac[16];
	char strMac[32]={0};
	if(get_local_mac_address("eth0", (char *)mac) < 0)
	{
		bwclinetLog(LOG_ERR,"Failed to get eth0 mac addr.");
		return -1;
	}
	snprintf(strMac, sizeof(strMac), "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	time_t timer;
	time(&timer);
	struct tm* tm_info = localtime(&timer);
	char timebuf[26];
	strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	char szIp[16]={0};
	get_local_ip_address("eth0", szIp);
	bw_mqtt_miner_get_devs_info();
	bw_mqtt_miner_get_pools_info();
	bw_mqtt_miner_get_summary_info();
	json_object_set_new (objectmsg, "id", json_string("0"));
	json_object_set_new (objectmsg, "date", json_string(timebuf));
	char type[32]={0};
	sprintf(type,"%s",minerSysInfo.type);//读配置
	if (strcmp(preminerSysInfo.type,type)){	
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "type", json_string(type));
		sprintf(preminerSysInfo.type,"%s",type);
		sprintf(minerSysInfo.type,"%s",type);
	}

	json_object_set_new (objectmsg, "ip", json_string(szIp));
	sprintf(minerSysInfo.ip,"%s",szIp);
	
	json_object_set_new (objectmsg, "mac", json_string(strMac));
	sprintf(minerSysInfo.mac,"%s",strMac);

	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	if (minerRuningInfo.mhs == 0 && minerRuningInfo.avg_mhs == 0)
		sprintf(minerRuningInfo.status,"%s",DEV_STATUS4);
	else if (minerRuningInfo.mhs <= minerRuningInfo.mhs_threshould)
		sprintf(minerRuningInfo.status,"%s",DEV_STATUS3);
	else if (!strcmp(minerRuningInfo.status,"E"))
		sprintf(minerRuningInfo.status,"%s",DEV_STATUS2);
	else if (!strcmp(minerRuningInfo.status,"S"))
		sprintf(minerRuningInfo.status,"%s",DEV_STATUS0);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	
	if (strcmp(preminerSysInfo.status,minerRuningInfo.status)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "status", json_string(minerRuningInfo.status));
		sprintf(preminerSysInfo.status,"%s",minerRuningInfo.status);
	}

	char ipType[8]={0},version[32]={0};
	char *result1 = cmd_system("cat /etc/network/interfaces | grep dhcp");
	strcmp(result1,"") == 0 ? strcpy(ipType,"static") : strcpy(ipType,"dhcp");
	char *result2 = cmd_system("cat /usr/firm.conf | grep version");
	strcmp(result2,"") == 0 ? strcpy(version,"undefined") : strcpy(version,result2+strlen("version"));
	if (strcmp(preminerSysInfo.version,version)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "version", json_string(version));
		sprintf(preminerSysInfo.version,"%s",version);
		sprintf(minerSysInfo.version,"%s",version);
	}
	if (strcmp(preminerSysInfo.ip_type,ipType)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "ip_type", json_string(ipType));
		sprintf(preminerSysInfo.ip_type,"%s",ipType);
		sprintf(minerSysInfo.ip_type,"%s",ipType);
	}
	
	char szMask[16]={0},szGatway[16]={0},szdns1[16]={0},szdns2[16]={0};
	unsigned int dns1,dns2;
	get_local_mask_address("eth0", szMask); 
	get_local_gateway_address("eth0", szGatway);
	get_cfg_dns_address("/etc/resolv.conf",&dns1,&dns2);
	sprintf(szdns1,"%d.%d.%d.%d",(dns1 >> 24) & 0xff,
								(dns1 >> 16) & 0xff,
								(dns1 >> 8) & 0xff,
								(dns1 >> 0) & 0xff);
	sprintf(szdns2,"%d.%d.%d.%d",(dns2 >> 24) & 0xff,
								(dns2 >> 16) & 0xff,
								(dns2 >> 8) & 0xff,
								(dns2 >> 0) & 0xff);
	if (strcmp(preminerSysInfo.netmask,szMask)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "netmask", json_string(szMask));
		sprintf(preminerSysInfo.netmask,"%s",szMask);
		sprintf(minerSysInfo.netmask,"%s",szMask);
	}	
	if (strcmp(preminerSysInfo.gateway,szGatway)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "gateway", json_string(szGatway));
		sprintf(preminerSysInfo.gateway,"%s",szGatway);
		sprintf(minerSysInfo.gateway,"%s",szGatway);
	}
	if (strcmp(preminerSysInfo.dns,szdns1)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "dns", json_string(szdns1));
		sprintf(preminerSysInfo.dns,"%s",szdns1);
		sprintf(minerSysInfo.dns,"%s",szdns1);
	}
	if (preminerSysInfo.periodic_time != minerSysInfo.periodic_time){
		(*changed_cnt)++;
		char str_periodic_time[32]={0};
		sprintf(str_periodic_time,"%d",minerSysInfo.periodic_time);
		json_object_set_new (objectmsg, "periodic_time", json_string(str_periodic_time));
		preminerSysInfo.periodic_time = minerSysInfo.periodic_time;
	}	

	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	if (!strcmp(minerSysInfo.pool1_status,"Alive"))
		sprintf(minerSysInfo.pool1_status,"%s",POOLS_STAUTS0);
	else if (!strcmp(minerSysInfo.pool1_status,"Unused"))
		sprintf(minerSysInfo.pool1_status,"%s",POOLS_STAUTS1);
	else if (!strcmp(minerSysInfo.pool1_status,"Alive") && minerSysInfo.pool1_diff == 0)
		sprintf(minerSysInfo.pool1_status,"%s",POOLS_STAUTS2);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
		
	if (strcmp(preminerSysInfo.pool1_status,minerSysInfo.pool1_status)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool1_status", json_string(minerSysInfo.pool1_status));
		sprintf(preminerSysInfo.pool1_status,"%s",minerSysInfo.pool1_status);
	}
	if (strcmp(preminerSysInfo.pool1_addr,minerSysInfo.pool1_addr)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool1_addr", json_string(minerSysInfo.pool1_addr));
		sprintf(preminerSysInfo.pool1_addr,"%s",minerSysInfo.pool1_addr);
	}
	if (strcmp(preminerSysInfo.pool1_miner_addr,minerSysInfo.pool1_miner_addr)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool1_miner_addr", json_string(minerSysInfo.pool1_miner_addr));
		sprintf(preminerSysInfo.pool1_miner_addr,"%s",minerSysInfo.pool1_miner_addr);
	}
	if (strcmp(preminerSysInfo.pool1_password,minerSysInfo.pool1_password)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool1_password", json_string(minerSysInfo.pool1_password));
		sprintf(preminerSysInfo.pool1_password,"%s",minerSysInfo.pool1_password);
	}

	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	if (!strcmp(minerSysInfo.pool2_status,"Alive"))
		sprintf(minerSysInfo.pool2_status,"%s",POOLS_STAUTS0);
	else if (!strcmp(minerSysInfo.pool2_status,"Unused"))
		sprintf(minerSysInfo.pool2_status,"%s",POOLS_STAUTS1);
	else if (!strcmp(minerSysInfo.pool2_status,"Alive") && minerSysInfo.pool2_diff == 0)
		sprintf(minerSysInfo.pool2_status,"%s",POOLS_STAUTS2);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	
	if (strcmp(preminerSysInfo.pool2_status,minerSysInfo.pool2_status)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool2_status", json_string(minerSysInfo.pool2_status));
		sprintf(preminerSysInfo.pool2_status,"%s",minerSysInfo.pool2_status);
	}
	if (strcmp(preminerSysInfo.pool2_addr,minerSysInfo.pool2_addr)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool2_addr", json_string(minerSysInfo.pool2_addr));
		sprintf(preminerSysInfo.pool2_addr,"%s",minerSysInfo.pool2_addr);
	}
	if (strcmp(preminerSysInfo.pool2_miner_addr,minerSysInfo.pool2_miner_addr)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool2_miner_addr", json_string(minerSysInfo.pool2_miner_addr));
		sprintf(preminerSysInfo.pool2_miner_addr,"%s",minerSysInfo.pool2_miner_addr);
	}
	if (strcmp(preminerSysInfo.pool2_password,minerSysInfo.pool2_password)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool2_password", json_string(minerSysInfo.pool2_password));
		sprintf(preminerSysInfo.pool2_password,"%s",minerSysInfo.pool2_password);
	}

	pthread_mutex_lock(&minerSysInfo.sysinfo_lock);
	if (!strcmp(minerSysInfo.pool3_status,"Alive"))
		sprintf(minerSysInfo.pool3_status,"%s",POOLS_STAUTS0);
	else if (!strcmp(minerSysInfo.pool3_status,"Unused"))
		sprintf(minerSysInfo.pool3_status,"%s",POOLS_STAUTS1);
	else if (!strcmp(minerSysInfo.pool3_status,"Alive") && minerSysInfo.pool3_diff == 0)
		sprintf(minerSysInfo.pool3_status,"%s",POOLS_STAUTS2);
	pthread_mutex_unlock(&minerSysInfo.sysinfo_lock);
	
	if (strcmp(preminerSysInfo.pool3_status,minerSysInfo.pool3_status)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool3_status", json_string(minerSysInfo.pool3_status));
		sprintf(preminerSysInfo.pool3_status,"%s",minerSysInfo.pool3_status);
	}
	if (strcmp(preminerSysInfo.pool3_addr,minerSysInfo.pool3_addr)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool3_addr", json_string(minerSysInfo.pool3_addr));
		sprintf(preminerSysInfo.pool3_addr,"%s",minerSysInfo.pool3_addr);
	}
	if (strcmp(preminerSysInfo.pool3_miner_addr,minerSysInfo.pool3_miner_addr)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool3_miner_addr", json_string(minerSysInfo.pool3_miner_addr));
		sprintf(preminerSysInfo.pool3_miner_addr,"%s",minerSysInfo.pool3_miner_addr);
	}
	if (strcmp(preminerSysInfo.pool3_password,minerSysInfo.pool3_password)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "pool3_password", json_string(minerSysInfo.pool3_password));
		sprintf(preminerSysInfo.pool3_password,"%s",minerSysInfo.pool3_password);
	}

	if (strcmp(preminerSysInfo.number,minerSysInfo.number)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "number", json_string(minerSysInfo.number));
		sprintf(preminerSysInfo.number,"%s",minerSysInfo.number);
	}
	
	if (strcmp(preminerSysInfo.position,minerSysInfo.position)){
		(*changed_cnt)++;
		json_object_set_new (objectmsg, "position", json_string(minerSysInfo.position));
		sprintf(preminerSysInfo.position,"%s",minerSysInfo.position);
	}

	char mhs[32] = {0},avg_mhs[32]={0},duration[32]={0};
	if (preminerRuningInfo.duration != minerRuningInfo.duration && abs(preminerRuningInfo.duration - minerRuningInfo.duration) >= minerSysInfo.periodic_time*60)//s
	{
		(*changed_cnt)++;
		sprintf(duration,"%lu",minerRuningInfo.duration);
		preminerRuningInfo.duration = minerRuningInfo.duration;
		json_object_set_new (objectmsg, "duration", json_string(duration));
	}

	if ((preminerRuningInfo.mhs != minerRuningInfo.mhs && abs(preminerRuningInfo.mhs - minerRuningInfo.mhs) > 50) || (*changed_cnt) > 0)
	{
		(*changed_cnt)++;
		sprintf(mhs,"%d",minerRuningInfo.mhs);
		preminerRuningInfo.mhs = minerRuningInfo.mhs;
		json_object_set_new (objectmsg, "mhs", json_string(mhs));
	}

	if ((preminerRuningInfo.avg_mhs != minerRuningInfo.avg_mhs && abs(preminerRuningInfo.avg_mhs - minerRuningInfo.avg_mhs) > 50) || (*changed_cnt) > 0)
	{
		(*changed_cnt)++;
		sprintf(avg_mhs,"%d",minerRuningInfo.avg_mhs);
		preminerRuningInfo.avg_mhs = minerRuningInfo.avg_mhs;
		json_object_set_new (objectmsg, "avg_mhs", json_string(avg_mhs));
	}

	
	result = json_dumps(objectmsg, JSON_PRESERVE_ORDER);
	if (0 != *changed_cnt)
		memcpy(jsonMsg,result,strlen(result));
	free(result);
	json_decref(objectmsg);
	return 0;
}

int bw_miner_runing_info_publish_msg(char *jsonMsg)
{
	if (!jsonMsg){
		bwclinetLog(LOG_ERR,"%s invaild input params.",__func__);
		return -1;
	}
	json_t *objectmsg;
	char *result;
	objectmsg = json_object();
	unsigned char mac[16];
	char strMac[32];
	time_t timer;
	time(&timer);
	struct tm* tm_info = localtime(&timer);
	char timebuf[26]={0};
	strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	
	if(get_local_mac_address("eth0", (char *)mac) < 0)
	{
		bwclinetLog(LOG_ERR,"Failed to get eth0 mac addr.");
		return -1;
	}
	snprintf(strMac, sizeof(strMac), "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	char szIp[32]={0};
	get_local_ip_address("eth0", szIp);
	
	json_object_set_new (objectmsg, "id", json_string("0"));
	json_object_set_new (objectmsg, "date", json_string(timebuf));
	json_object_set_new (objectmsg, "ip", json_string(szIp));
	json_object_set_new (objectmsg, "mac", json_string(strMac));
	sprintf(minerRuningInfo.ip,"%s",szIp);
	sprintf(minerRuningInfo.mac,"%s",strMac);

	bw_mqtt_miner_get_devs_info();
	bw_mqtt_miner_get_summary_info();
	bw_mqtt_miner_get_pools_info();

	if (0 == strcmp(minerSysInfo.type,L61)){
		bw_mqtt_miner_get_boards_info();
	}
	
	char mhs[32] = {0},avg_mhs[32]={0},accept[32] = {0},reject[32] = {0},diff[32]={0},temperature[32]={0};
	minerRuningInfo.mhs = minerRuningInfo.board1_mhs+minerRuningInfo.board2_mhs+minerRuningInfo.board3_mhs+minerRuningInfo.board4_mhs;
	sprintf(mhs,"%d",minerRuningInfo.mhs);
	minerRuningInfo.avg_mhs = minerRuningInfo.board1_avg_mhs+minerRuningInfo.board2_avg_mhs+minerRuningInfo.board3_avg_mhs+minerRuningInfo.board4_avg_mhs;
	sprintf(avg_mhs,"%d",minerRuningInfo.avg_mhs);
	minerRuningInfo.accept = minerRuningInfo.board1_accept+minerRuningInfo.board2_accept+minerRuningInfo.board3_accept+minerRuningInfo.board4_accept;
	sprintf(accept,"%d",minerRuningInfo.accept);
	minerRuningInfo.reject = minerRuningInfo.board1_reject+minerRuningInfo.board2_reject+minerRuningInfo.board3_reject+minerRuningInfo.board4_reject;
	sprintf(reject,"%d",minerRuningInfo.reject);
	minerRuningInfo.diff = 
						minerSysInfo.pool1_diff == 0 ? 
						(minerSysInfo.pool2_diff == 0 ? minerSysInfo.pool3_diff : minerSysInfo.pool2_diff) : 
						minerSysInfo.pool1_diff;
	sprintf(diff,"%d",minerRuningInfo.diff);

	minerRuningInfo.temperature = minerRuningInfo.board1_temperate == 0 ? 
								(minerRuningInfo.board2_temperate == 0 ? 
								(minerRuningInfo.board3_temperate == 0 ?
								minerRuningInfo.board4_temperate :
								minerRuningInfo.board3_temperate) :
								minerRuningInfo.board2_temperate):
								minerRuningInfo.board1_temperate;

	sprintf(temperature,"%d",minerRuningInfo.temperature);

	json_object_set_new (objectmsg, "mhs", json_string(mhs));
	json_object_set_new (objectmsg, "avg_mhs", json_string(avg_mhs));
	json_object_set_new (objectmsg, "accept", json_string(accept));
	json_object_set_new (objectmsg, "reject", json_string(reject));
	json_object_set_new (objectmsg, "diff", json_string(diff));
	json_object_set_new (objectmsg, "temperature", json_string(temperature));
	json_object_set_new (objectmsg, "boards", json_string("4"));

	//board1 info
	char board1_mhs[32]={0},board1_temperate[32]={0},board1_frequency[32]={0},board1_accept[32]={0},board1_reject[32]={0};
	sprintf(board1_mhs,"%d",minerRuningInfo.board1_mhs);
	sprintf(board1_temperate,"%d",minerRuningInfo.board1_temperate);
	sprintf(board1_frequency,"%d",minerRuningInfo.board1_frequency);
	sprintf(board1_accept,"%d",minerRuningInfo.board1_accept);
	sprintf(board1_reject,"%d",minerRuningInfo.board1_reject);

	json_object_set_new (objectmsg, "board1_mhs", json_string(board1_mhs));
	json_object_set_new (objectmsg, "board1_temperate", json_string(board1_temperate));
	json_object_set_new (objectmsg, "board1_frequency", json_string(board1_frequency));
	json_object_set_new (objectmsg, "board1_accept", json_string(board1_accept));
	json_object_set_new (objectmsg, "board1_reject", json_string(board1_reject));
	
	//board2 info
	char board2_mhs[32]={0},board2_temperate[32]={0},board2_frequency[32]={0},board2_accept[32]={0},board2_reject[32]={0};
	sprintf(board2_mhs,"%d",minerRuningInfo.board1_mhs);
	sprintf(board2_temperate,"%d",minerRuningInfo.board2_temperate);
	sprintf(board2_frequency,"%d",minerRuningInfo.board2_frequency);
	sprintf(board2_accept,"%d",minerRuningInfo.board2_accept);
	sprintf(board2_reject,"%d",minerRuningInfo.board2_reject);

	json_object_set_new (objectmsg, "board2_mhs", json_string(board2_mhs));
	json_object_set_new (objectmsg, "board2_temperate", json_string(board2_temperate));
	json_object_set_new (objectmsg, "board2_frequency", json_string(board2_frequency));
	json_object_set_new (objectmsg, "board2_accept", json_string(board2_accept));
	json_object_set_new (objectmsg, "board2_reject", json_string(board2_reject));
	

	//board3 info
	char board3_mhs[32]={0},board3_temperate[32]={0},board3_frequency[32]={0},board3_accept[32]={0},board3_reject[32]={0};
	sprintf(board3_mhs,"%d",minerRuningInfo.board3_mhs);
	sprintf(board3_temperate,"%d",minerRuningInfo.board3_temperate);
	sprintf(board3_frequency,"%d",minerRuningInfo.board3_frequency);
	sprintf(board3_accept,"%d",minerRuningInfo.board3_accept);
	sprintf(board3_reject,"%d",minerRuningInfo.board3_reject);

	json_object_set_new (objectmsg, "board3_mhs", json_string(board3_mhs));
	json_object_set_new (objectmsg, "board3_temperate", json_string(board3_temperate));
	json_object_set_new (objectmsg, "board3_frequency", json_string(board3_frequency));
	json_object_set_new (objectmsg, "board3_accept", json_string(board3_accept));
	json_object_set_new (objectmsg, "board3_reject", json_string(board3_reject));
	

	//board4 info
	char board4_mhs[32]={0},board4_temperate[32]={0},board4_frequency[32]={0},board4_accept[32]={0},board4_reject[32]={0};
	sprintf(board4_mhs,"%d",minerRuningInfo.board4_mhs);
	sprintf(board4_temperate,"%d",minerRuningInfo.board4_temperate);
	sprintf(board4_frequency,"%d",minerRuningInfo.board4_frequency);
	sprintf(board4_accept,"%d",minerRuningInfo.board4_accept);
	sprintf(board4_reject,"%d",minerRuningInfo.board4_reject);

	json_object_set_new (objectmsg, "board4_mhs", json_string(board4_mhs));
	json_object_set_new (objectmsg, "board4_temperate", json_string(board4_temperate));
	json_object_set_new (objectmsg, "board4_frequency", json_string(board4_frequency));
	json_object_set_new (objectmsg, "board4_accept", json_string(board4_accept));
	json_object_set_new (objectmsg, "board4_reject", json_string(board4_reject));
	
	char fan1_status[16]={0},fan1_speed[16]={0},fan2_status[16]={0},fan2_speed[16]={0};
	char *result1 = cmd_system("cat /sys/class/pwm/pwmchip0/pwm1/enable");
	if (atoi(result1) == 1){
		strncpy(fan1_status,FANS_STAUTS0,sizeof(fan1_status));
	}else{
		strncpy(fan1_status,FANS_STAUTS1,sizeof(fan1_status));
	}
#if 0	
	char *result2 = cmd_system("cat /sys/class/pwm/pwmchip0/pwm1/period");
	sprintf(fan1_speed,"%d",atoi(result2));

	//L21 two fans control by pwm1
	strncpy(fan2_status,fan1_status,sizeof(fan2_status));
	strncpy(fan2_speed,fan1_speed,sizeof(fan2_speed));
#endif
	sprintf(fan1_speed,"%d",minerRuningInfo.fan1_speed);
	sprintf(fan2_speed,"%d",minerRuningInfo.fan2_speed);

	if (atoi(result1) == 1 && fan1_speed == 0)
		sprintf(fan1_status,"%s",FANS_STAUTS2);
	
	//L21 two fans control by pwm1
	strncpy(fan2_status,fan1_status,sizeof(fan2_status));
	
	json_object_set_new (objectmsg, "fans", json_string("2"));
	json_object_set_new (objectmsg, "fan1_status", json_string(fan1_status));
	json_object_set_new (objectmsg, "fan1_speed", json_string(fan1_speed));
	json_object_set_new (objectmsg, "fan2_status", json_string(fan2_status));
	json_object_set_new (objectmsg, "fan2_speed", json_string(fan2_speed));
	minerRuningInfo.fans = 2;
	sprintf(minerRuningInfo.fan1_status,"%s",fan1_status);
	sprintf(minerRuningInfo.fan2_status,"%s",fan2_status);
	
	char str_duration[32]={0};
	sprintf(str_duration,"%lu",minerRuningInfo.duration);
	json_object_set_new (objectmsg, "duration", json_string(str_duration));
	
	char str_temperature_threshould[8]={0};
	sprintf(str_temperature_threshould,"%d",minerRuningInfo.temperature_threshould);
	json_object_set_new (objectmsg, "temperature_threshould", json_string(str_temperature_threshould));

	char str_mhs_threshould[8]={0};
	sprintf(str_mhs_threshould,"%d",minerRuningInfo.mhs_threshould);
	json_object_set_new (objectmsg, "mhs_threshould", json_string(str_mhs_threshould));
	
	result = json_dumps(objectmsg, JSON_PRESERVE_ORDER);
	memcpy(jsonMsg,result,strlen(result));	
	free(result);
	json_decref(objectmsg);
	return 0;
}

int bw_miner_set_network(int flag)
{
	if (0 == strcmp(miner_cfg.ip_type,"dhcp"))
	{
		//set hdcp
		system("rm -f /etc/network/interfaces");
		system("echo \"auto lo\" >> /etc/network/interfaces");
		system("echo \"auto eth0\" >> /etc/network/interfaces");
		system("echo \"iface lo inet loopback\" >> /etc/network/interfaces");
		system("echo \"iface eth0 inet dhcp\" >> /etc/network/interfaces"); //dhcp
		system("chmod 766 /etc/network/interfaces");

		//clear dns(later dhcp-client will re-write it.)
		system("rm -f /etc/resolv.conf");
		system("echo > /etc/resolv.conf");
		system("chmod 766 /etc/resolv.conf");

		system("sync");
		//system("ifdown eth0 && ifup eth0");
		system("killall -9 cgminer");
		system("killall -9 mqtt_client");
		system("sync");
		//reboot
		system("reboot");
	}
	else
	{
		if (!miner_cfg.ip || !miner_cfg.netmask || !miner_cfg.gateway || !miner_cfg.dns)
				return -1;

		char cmdip[128]={0},cmdmask[128]={0},cmdgw[128]={0},cmddns[128]={0};
		if(!flag)
			sprintf(cmdip, "echo \"address %s\" >> /etc/network/interfaces", miner_cfg.ip);//批量设置不设置静态ip
		sprintf(cmdmask, "echo \"netmask %s\" >> /etc/network/interfaces", miner_cfg.netmask);
		sprintf(cmdgw, "echo \"gateway %s\" >> /etc/network/interfaces", miner_cfg.gateway);
		sprintf(cmddns, "echo \"nameserver %s\" >> /etc/resolv.conf", miner_cfg.dns);
		//set ip gw mask
		system("rm -f /etc/network/interfaces");
		system("echo \"auto lo\" >> /etc/network/interfaces");
		system("echo \"auto eth0\" >> /etc/network/interfaces");
		system("echo \"iface lo inet loopback\" >> /etc/network/interfaces");
		system("echo \"iface eth0 inet static\" >> /etc/network/interfaces");
		if (!flag)
			system(cmdip);
		system(cmdmask);
		system(cmdgw);
		system("chmod 766 /etc/network/interfaces");
		//set dns
		system("rm -f /etc/resolv.conf");
		system(cmddns);
		system("chmod 766 /etc/resolv.conf");

		system("sync");
		//system("/etc/init.d/networking restart");
		//system("ifdown eth0 && ifup eth0");
		system("killall -9 cgminer");
		system("killall -9 mqtt_client");
		system("sync");
		//reboot
		system("reboot");
	}
    return 0;
}
int bw_miner_set_pools()
{
	char cmd1[128]={0},cmd2[128]={0},cmd3[128]={0};
	sprintf(cmd1,"/usr/app/mqtt_client_cfgset.sh %d %s %s %s",SET_POOL1,miner_cfg.pool1_addr,miner_cfg.pool1_miner_addr,miner_cfg.pool1_password);
	system(cmd1);
	bwclinetLog(LOG_INFO,"%s cmd1=%s",__func__,cmd1);
	sprintf(cmd2,"/usr/app/mqtt_client_cfgset.sh %d %s %s %s",SET_POOL2,miner_cfg.pool2_addr,miner_cfg.pool2_miner_addr,miner_cfg.pool2_password);
	system(cmd2);
	bwclinetLog(LOG_INFO,"%s cmd2=%s",__func__,cmd2);
	sprintf(cmd3,"/usr/app/mqtt_client_cfgset.sh %d %s %s %s",SET_POOL3,miner_cfg.pool3_addr,miner_cfg.pool3_miner_addr,miner_cfg.pool3_password);
	system(cmd3);
	bwclinetLog(LOG_INFO,"%s cmd3=%s",__func__,cmd3);
	system("killall -9 cgminer");
	usleep(1000000UL);
	system("ifdown eth0");
	usleep(2000000UL);
	system("ifup eth0");
	//system("/usr/app/cgminer &");
	system("/etc/init.d/*cgminer restart");
	return 0;
}
int bw_miner_set_position()
{
	char cmd[128]={0};
	sprintf(cmd,"/usr/app/mqtt_client_cfgset.sh %d %s",SET_POSITION,miner_cfg.position);
	bwclinetLog(LOG_INFO,"%s cmd=%s",__func__,cmd);
	system(cmd);
	return 0;
}
int bw_miner_set_number()
{
	char cmd[128]={0};
	sprintf(cmd,"/usr/app/mqtt_client_cfgset.sh %d %s",SET_NUMBER,miner_cfg.number);
	bwclinetLog(LOG_INFO,"%s cmd=%s",__func__,cmd);
	system(cmd);
	return 0;
}
int bw_miner_set_threshould()
{
	char cmd1[128]={0},cmd2[128]={0};
	sprintf(cmd1,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_TEMPERATURE_THRESHOULD,miner_cfg.temp_threshould);
	system(cmd1);
	bwclinetLog(LOG_INFO,"%s cmd1=%s",__func__,cmd1);
	sprintf(cmd2,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_MHS_THRESHOULD,miner_cfg.mhs_threshould);
	system(cmd2);
	bwclinetLog(LOG_INFO,"%s cmd2=%s",__func__,cmd2);
	return 0;
}
int bw_miner_set_fan_speed()
{
	char cmd1[128]={0},cmd2[128]={0},cmd3[128]={0},cmd4[128]={0};
	sprintf(cmd1,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FAN1_SPEED,miner_cfg.fan_speed[0]);
	system(cmd1);
	bwclinetLog(LOG_INFO,"%s cmd1=%s",__func__,cmd1);
	sprintf(cmd2,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FAN2_SPEED,miner_cfg.fan_speed[1]);
	system(cmd2);
	bwclinetLog(LOG_INFO,"%s cmd2=%s",__func__,cmd2);
	sprintf(cmd3,"/usr/app/minerapi fan fan1_speed=%d",miner_cfg.fan_speed[0]);
	system(cmd3);
	bwclinetLog(LOG_INFO,"%s cmd3=%s",__func__,cmd3);
	sprintf(cmd4,"/usr/app/minerapi fan fan2_speed=%d",miner_cfg.fan_speed[1]);
	system(cmd4);
	bwclinetLog(LOG_INFO,"%s cmd4=%s",__func__,cmd4);
	return 0;
}

int bw_miner_set_perioidc_time()
{
	char cmd[128]={0};
	sprintf(cmd,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_PERIODIC_TIME,miner_cfg.periodic_time);
	bwclinetLog(LOG_INFO,"%s cmd=%s",__func__,cmd);
	system(cmd);
	return 0;
}
int bw_miner_set_frequency()
{
	char cmd0[8]={0},cmd1[128]={0},cmd2[128]={0},cmd3[128]={0},cmd4[128]={0};
	if (miner_cfg.use_frequency_all == true){
		sprintf(cmd0,"/usr/app/mqtt_client_cfgset.sh %d true",SET_USE_FREQUENCY_ALL);
		system(cmd0);
		bwclinetLog(LOG_INFO,"%s cmd0=%s",__func__,cmd0);
		sprintf(cmd1,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FREQUENCY_ALL,miner_cfg.frequency_all);
		system(cmd1);
		bwclinetLog(LOG_INFO,"%s cmd1=%s",__func__,cmd1);		
	}else{
		sprintf(cmd0,"/usr/app/mqtt_client_cfgset.sh %d false",SET_USE_FREQUENCY_ALL);
		system(cmd0);
		bwclinetLog(LOG_INFO,"%s cmd0=%s",__func__,cmd0);
		sprintf(cmd1,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FREQUENCY1,miner_cfg.frequency_1);
		system(cmd1);
		bwclinetLog(LOG_INFO,"%s cmd1=%s",__func__,cmd1);
		sprintf(cmd2,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FREQUENCY2,miner_cfg.frequency_2);
		system(cmd2);
		bwclinetLog(LOG_INFO,"%s cmd2=%s",__func__,cmd2);
		sprintf(cmd3,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FREQUENCY3,miner_cfg.frequency_3);
		system(cmd3);
		bwclinetLog(LOG_INFO,"%s cmd3=%s",__func__,cmd3);
		sprintf(cmd4,"/usr/app/mqtt_client_cfgset.sh %d %d",SET_FREQUENCY4,miner_cfg.frequency_4);
		system(cmd4);
		bwclinetLog(LOG_INFO,"%s cmd4=%s",__func__,cmd4);
	}
	system("killall -9 cgminer");	
	usleep(1000000UL);
	system("ifdown eth0");
	usleep(2000000UL);
	system("ifup eth0");
	//system("/usr/app/cgminer &");
	system("/etc/init.d/*cgminer restart");
	return 0;
}


int upgrade_uboot()
{
	if(access("/tmp/MLO",F_OK)==0)
	{
		system("/usr/sbin/flash_erase /dev/mtd0 0 0 >/dev/null 2>&1");
		system("/usr/sbin/nandwrite -p /dev/mtd0 /tmp/MLO >/dev/null 2>&1");
		bwclinetLog(LOG_INFO,"upgrade MLO sucess.");
	}
	if(access("/tmp/u-boot.img",F_OK)==0)
	{		
		system("/usr/sbin/flash_erase /dev/mtd4 0 0 >/dev/null 2>&1");
		system("/usr/sbin/nandwrite -p /dev/mtd4 /tmp/u-boot.img >/dev/null 2>&1");
		bwclinetLog(LOG_INFO,"upgrade u-boot.img sucess.");
	}
	return 0;
}

int upgrade_kernel()
{
	if(access("/tmp/zImage",F_OK)==0)
	{
		system("/usr/sbin/flash_erase /dev/mtd6 0 0 >/dev/null 2>&1");
		system("/usr/sbin/nandwrite -p /dev/mtd6 /tmp/zImage >/dev/null 2>&1");
		bwclinetLog(LOG_INFO,"upgrade kernel zImage sucess.");
	}
	if(access("/tmp/uImage",F_OK)==0)
	{
		system("/usr/sbin/flash_erase /dev/mtd6 0 0 >/dev/null 2>&1");
		system("/usr/sbin/nandwrite -p /dev/mtd6 /tmp/uImage >/dev/null 2>&1");
		bwclinetLog(LOG_INFO,"upgrade kernel uImage sucess.");
	}
	return 0;
}
int upgrade_dtb()
{
	if(access("/tmp/am335x-boneblack.dtb",F_OK)==0)
	{
		system("/usr/sbin/flash_erase /dev/mtd3 0 0 >/dev/null 2>&1");
		system("/usr/sbin/nandwrite -p /dev/mtd3 /tmp/am335x-boneblack.dtb >/dev/null 2>&1");
		bwclinetLog(LOG_INFO,"upgrade dtb sucess.");
		
	}
	return 0;
}
int upgrade_rootfs()
{
	char cmd[128]={0};
	int num = 0;
	char str_new[64]={0};
	char *str_old = cmd_system("/usr/sbin/fw_printenv nandroot");
	if (str_old)
	{
		char *p1 = strstr(str_old,"ubi.mtd=8");
		char *p2 = strstr(str_old,"ubi.mtd=7");
		if(p1)
		{
			sprintf(str_new,"%s","ubi0:rootfs rw ubi.mtd=7,2048");
			num = 7;
		}
		if (p2)
		{
			sprintf(str_new,"%s","ubi0:rootfs rw ubi.mtd=8,2048");
			num = 8;		
		}
	}
	bwclinetLog(LOG_INFO,"num=%d str_new=%s",num,str_new);
	if(access("/tmp/rootfs.img",F_OK)==0)
	{
		if (num == 7)
		{
			system("/usr/sbin/flash_erase /dev/mtd7 0 0 >/dev/null 2>&1");
			system("/usr/sbin/ubiformat /dev/mtd7 -f /tmp/rootfs.img -s 512 -O 2048 >/dev/null 2>&1");
		}
		else if (num == 8)
		{
			system("/usr/sbin/flash_erase /dev/mtd8 0 0 >/dev/null 2>&1");
			system("/usr/sbin/ubiformat /dev/mtd8 -f /tmp/rootfs.img -s 512 -O 2048 >/dev/null 2>&1");
		}
		sprintf(cmd,"/usr/sbin/fw_setenv nandroot %s >/dev/null 2>&1",str_new);
		system(cmd);
		bwclinetLog(LOG_INFO,"upgrade rootfs.img sucess. cmd=%s",cmd);
	}else{
		return -1;
	}
	return 0;
}  

int bw_exec_update_firm()
{
	if(access(UPDATE_FILE_PATH,F_OK)==0)
	{
		system("cd /tmp/ && tar -xvf miner_tool_update.tar.gz");
		system("killall -9 cgminer");
		system("killall -9 bwminer");
		upgrade_uboot();
		usleep(1000000UL);
		upgrade_dtb();
		usleep(1000000UL);
		upgrade_kernel();
		usleep(1000000UL);
		upgrade_rootfs();
		usleep(2000000UL);
		bwclinetLog(LOG_INFO,"bw_exec_update_firm done!");
	}else{
		bwclinetLog(LOG_INFO,"not find update.tar.gz file!");
		return -1;
	}
	return 0;
}
int bw_miner_update_publish_msg(char *jsonMsg,double progress)
{
	if (!jsonMsg){
		bwclinetLog(LOG_ERR,"%s invaild input params.",__func__);
		return -1;
	}
	json_t *objectmsg;
	char *result;
	objectmsg = json_object();
	unsigned char mac[16];
	char strMac[32];
	if(get_local_mac_address("eth0", (char *)mac) < 0)
	{
		bwclinetLog(LOG_ERR,"Failed to get eth0 mac addr.");
		return -1;
	}
	snprintf(strMac, sizeof(strMac), "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	json_object_set_new (objectmsg, "mac", json_string(strMac));
	char szprogress[8]={0};
	sprintf(szprogress,"%d",(int)progress);
	json_object_set_new (objectmsg, "progress", json_string(szprogress));
	result = json_dumps(objectmsg, JSON_PRESERVE_ORDER);
	memcpy(jsonMsg,result,strlen(result));
	free(result);
	json_decref(objectmsg);
	return 0;
}

int bw_miner_publish_update_progress(struct mqtt_client *client,double progress)
{
	if (!client)
		return -1;
	
	char topic[128] = {0};
	char jsonMsg[1024]={0};

	memset(jsonMsg,0,sizeof(jsonMsg));
	bw_miner_update_publish_msg(jsonMsg,progress);
	sprintf(topic,"%s",TOPIC_UPDATE_PROGRESS);

	/* publish update progress*/
	mqtt_publish(client, topic, jsonMsg, strlen(jsonMsg), MQTT_PUBLISH_QOS_1);
	bwclinetLog(LOG_INFO,"%s %s",__func__,jsonMsg);

	/* check for errors */
	if (client->error != MQTT_OK) {
		fprintf(stderr, "error: %s\n", mqtt_error_str(client->error));
		bwclinetLog(LOG_ERR,"error: %s", mqtt_error_str(client->error));
		exit_mqtt_client(EXIT_FAILURE, client->socketfd, &client_daemon);
	}
	usleep(200000UL);//200ms
	return 0;
}

/*for libcurl newer than 7.32.0*/
static int progress_callback(void *userdata, curl_off_t dltotal, 
						curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    (void)ultotal;
    (void)ulnow;
	myprogress *data = (myprogress *)userdata;
	
    if(quit)
        return 1;

    time_t now = time(NULL);
	if (now - data->lastTime < 1)
	{
		return 0;
	}
	data->lastTime = now;
	CURL *easy_handle = data->curl;

	// Defaults to bytes/second
	double speed = 0;
	curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed);

	// Time remaining
	//double leftTime = 0;

	// Progress percentage
	double progress = 0;

	if (dltotal != 0 && speed != 0)
	{
		progress = (dlnow + data->resumeByte) / data->downloadFileLength * 100;
		//leftTime = (data->downloadFileLength - dlnow - data->resumeByte) / speed;
		bwclinetLog(LOG_INFO,"downloading firm percent:\t%.2f%%", progress);
		bw_miner_publish_update_progress(data->client,progress);
	}
	return 0;
}
size_t nousecb(char *buffer, size_t x, size_t y, void *userdata)
{
	(void)buffer;
	(void)userdata;
	return x * y;
}
// Get the file size on the server
double getDownloadFileLength(const char *url)
{
    CURL *easy_handle = NULL;
	int ret = CURLE_OK;
	double size = -1;
	do
	{
		easy_handle = curl_easy_init();
		if (!easy_handle)
		{
            bwclinetLog(LOG_ERR,"curl_easy_init error");
			break;
		}

		// Only get the header data
		ret = curl_easy_setopt(easy_handle, CURLOPT_URL, url);
		ret |= curl_easy_setopt(easy_handle, CURLOPT_HEADER, 1L);
		ret |= curl_easy_setopt(easy_handle, CURLOPT_NOBODY, 1L);
        ret |= curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, nousecb);	// libcurl_a.lib will return error code 23 without this sentence on windows

		if (ret != CURLE_OK)
		{
            bwclinetLog(LOG_ERR,"curl_easy_setopt error");
			break;
		}

		ret = curl_easy_perform(easy_handle);
		if (ret != CURLE_OK)
		{
			char s[100] = {0};
			sprintf(s, "error:%d:%s", ret, curl_easy_strerror(ret));
			bwclinetLog(LOG_ERR,"===%s",s);
			break;
		}
		// size = -1 if no Content-Length return or Content-Length=0
		ret = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
		if (ret != CURLE_OK)
		{
            bwclinetLog(LOG_ERR,"curl_easy_getinfo error");
			break;
		}
	} while (0);
	curl_easy_cleanup(easy_handle);
	return size;
}

// Get the local file size, return -1 if failed
int getLocalFileLength(const char* path)
{
	int ret;
	struct stat fileStat;

	ret = stat(path, &fileStat);
	if (ret == 0)
	{
		return fileStat.st_size;
	}
	return ret;
}
size_t process_data(void *buffer, size_t size, size_t nmemb, void *user_p) 
{ 	
	FILE *fp = (FILE *)user_p; 	
	size_t return_size = fwrite(buffer, size, nmemb, fp); 	
	return return_size; 
}

int bw_miner_update(struct mqtt_client *client)
{
	if (!client)
		return -1;
	
	myprogress prog;
	CURLcode res = CURLE_OK;
	char url[128]={0};
	char path[128]={0};
	sprintf(url,"%s",miner_cfg.update_url);//要下载的升级包远程路径
	sprintf(path,"%s",UPDATE_FILE_PATH);//下载升级包保存的路径

	prog.downloadFileLength = getDownloadFileLength(url);
	if(prog.downloadFileLength < 0)
	{
		bwclinetLog(LOG_ERR,"getDownloadFileLength error");
		return -1;
	}
	bwclinetLog(LOG_INFO,"downloadFileLength=%.2f",prog.downloadFileLength);
	// 获取easy handle	
	CURL *easy_handle = curl_easy_init();
	if (NULL == easy_handle)
	{
		bwclinetLog(LOG_ERR,"get a easy handle failed.");
		curl_global_cleanup(); 
		return -1;
	}
	prog.lastTime = 0;
	prog.curl = easy_handle;
	prog.client = client;

	FILE *fp = fopen(path, "ab+");

	// 设置easy handle属性
	curl_easy_setopt(easy_handle, CURLOPT_URL, url);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &process_data);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);

	curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, progress_callback);
	curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, &prog);
	curl_easy_setopt(easy_handle, CURLOPT_FAILONERROR, 1L);  
	curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, 10L);

	prog.resumeByte = getLocalFileLength(path);
	if (prog.resumeByte > 0)
	{
		// Set a point to resume transfer
		res |= curl_easy_setopt(easy_handle, CURLOPT_RESUME_FROM_LARGE, prog.resumeByte);
	}	
	//bwclinetLog(LOG_INFO,"resumeByte=%l",prog.resumeByte);
	// 执行数据请求	
	res = curl_easy_perform(easy_handle);	
	if(res != CURLE_OK)
	  fprintf(stderr, "%s\n", curl_easy_strerror(res));
	// 释放资源 
	fclose(fp);
	curl_easy_cleanup(easy_handle);
	if(!bw_exec_update_firm())
	{
		bwclinetLog(LOG_INFO,"bw_miner_update sucess!");
		bw_miner_publish_update_progress(client,100.00);
		usleep(2000000UL);
		system("reboot");
	}
	else
		bwclinetLog(LOG_ERR,"bw_miner_update failed!");

	return 0;
}

int bw_miner_set_reboot()
{
	char cmd[128]={0};
	sprintf(cmd,"reboot");
	bwclinetLog(LOG_INFO,"%s cmd=%s",__func__,cmd);
	system(cmd);
	return 0;	
}
bool bw_miner_get_update_flag()
{
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	bool flag = miner_cfg.update_report_flag;
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return flag;
}

int bw_miner_set_update_flag(bool flag)
{
	pthread_mutex_lock(&miner_cfg.cfginfo_lock);
	miner_cfg.update_report_flag = flag;
	pthread_mutex_unlock(&miner_cfg.cfginfo_lock);
	return 0;
}
int bw_miner_judge_batch_set_flag(int *batchsetflag)
{
	//判断本机IP是否在批量ip列表里面
	if (!batchsetflag)
		return -1;
	size_t i = 0;
	char ip_start[16]={0},ip_end[16]={0},ip_local[16]={0},ip_single[16]={0};
	unsigned long ip_s=0,ip_e=0,ip_l=0;
	char *p=NULL,*tmp=NULL;
	size_t miner_cfg_ip_array_szie = ARRAY_SIZE(miner_cfg.ipAarry);
	for (i=0; i<miner_cfg_ip_array_szie;++i)
	{
		if (strcmp(miner_cfg.ipAarry[i],"")){
			p = miner_cfg.ipAarry[i];
			tmp = strstr(p,"~");
			if (tmp){
				strncpy(ip_start,p,tmp-p);
				strncpy(ip_end,tmp+1,strlen(tmp)-1);
				ip_end[strlen(tmp)-1]='\0';
				ip_s = ntohl(inet_addr(ip_start));
				ip_e = ntohl(inet_addr(ip_end));
				get_local_ip_address("eth0", ip_local);
				ip_l = ntohl(inet_addr(ip_local));
				//bwclinetLog(LOG_INFO,"batch set,ip array index:%d ip_start:%s ip_end:%s ip_local:%s ip_s:%u ip_e:%u ip_l:%u",i,ip_start,ip_end,ip_local,ip_s,ip_e,ip_l);
				if (ip_l >= ip_s && ip_l <= ip_e){
					*batchsetflag = 1;
					break;
				}
			}else{
				strncpy(ip_single,p,strlen(p));
				get_local_ip_address("eth0", ip_local);
				if (!strcmp(ip_single,ip_local)){
					*batchsetflag = 1;
					break;
				}
			}
		}
	}
	return 0;
}

int bw_mqtt_miner_set_cfg(const char *topic_name,const char *cfg)
{
	//解析json data
	json_t *objectmsg = NULL;
	json_t *json_val;
	json_error_t json_err;
	size_t i = 0,j = 0,k = 0;
	int ret = -1;

	objectmsg = json_loadb(cfg,strlen(cfg), 0, &json_err);
	size_t miner_cfg_array_size = ARRAY_SIZE(Miner_cfg_options);
	if (json_is_object(objectmsg))
	{
		json_t *value;
		for (j=0;j<miner_cfg_array_size;++j)
		{
			if (!Miner_cfg_options[j].name)
				break;
			value = json_object_get(objectmsg, Miner_cfg_options[j].name);
			if(!value)
				continue;
			char *s = NULL;
			if (json_is_string(value)){
				s = strdup(json_string_value(value));
			}
			else if (json_is_number(value)){
				s = malloc(64);
				if(s){sprintf(s,"%d",(int)json_number_value(value));}
			}
			else if (json_is_object(value)){
				json_t *json_val;
				for (k=0;k<miner_cfg_array_size;++k)
				{
					if (!Miner_cfg_options[k].name)
						break;
					json_val = json_object_get(value, Miner_cfg_options[k].name);
					if(!json_val)
						continue;
					if (json_is_string(json_val)){
						s = strdup(json_string_value(json_val));
					}
					if (json_is_number(json_val)){
						s = malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(json_val));}
					}
					if (json_is_boolean(json_val)){
						s = (char *)malloc(8);
						if (json_is_true(json_val)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(json_val)){
							if(s){sprintf(s,"%s","false");}
						}
					}					
					if (!s)
						continue;
					if (Miner_cfg_options[k].func)
						Miner_cfg_options[k].func(s,i);
					s = NULL;			
				}
			}			
			else if (json_is_array(value)){
				size_t j_array_size = json_array_size(value);
				for (i=0; i< j_array_size;++i)
				{
					json_val = json_array_get(value,i);
					if (json_is_string(json_val)){
						s = strdup(json_string_value(json_val));
					}
					if (json_is_number(json_val)){
						s = malloc(64);
						if(s){sprintf(s,"%d",(int)json_number_value(json_val));}
					}
					if (json_is_boolean(json_val)){
						s = (char *)malloc(8);
						if (json_is_true(json_val)){
							if(s){sprintf(s,"%s","true");}
						}else if (json_is_false(json_val)){
							if(s){sprintf(s,"%s","false");}
						}
					}
					if (json_is_object(json_val)){
						json_t *value;
						for (k=0;k<miner_cfg_array_size;++k)
						{
							if (!Miner_cfg_options[k].name)
								break;
							value = json_object_get(json_val, Miner_cfg_options[k].name);
							if(!value)
								continue;
							if (json_is_string(value)){
								s = strdup(json_string_value(value));
							}
							if (json_is_number(value)){
								s = malloc(64);
								if(s){sprintf(s,"%d",(int)json_number_value(value));}
							}
							if (json_is_boolean(value)){
								s = (char *)malloc(8);
								if (json_is_true(value)){
									if(s){sprintf(s,"%s","true");}
								}else if (json_is_false(value)){
									if(s){sprintf(s,"%s","false");}
								}
							}					
							if (!s)
								continue;
							if (Miner_cfg_options[k].func)
								Miner_cfg_options[k].func(s,i);
							s = NULL;			
						}
					}
					if (!s)
						continue;
					if (Miner_cfg_options[j].func)
						Miner_cfg_options[j].func(s,i);
					s = NULL;
				}
			}
			if (!s)
				continue;
			if (Miner_cfg_options[j].func)
				ret = Miner_cfg_options[j].func(s,i);
			if (!strcmp(Miner_cfg_options[j].name,"mac") && ret == -1)
			{
				bwclinetLog(LOG_INFO,"mac not match!");
				free(s);
				break;
			}
			free(s);			
		}	
	}else{bwclinetLog(LOG_ERR,"%s it's not json data.",__func__);}
	json_decref(objectmsg);

	//单机设置
	if (miner_cfg.match == 1)
	{
		if (!strcmp(topic_name,TOPIC_SET_NETWORK)){
			bwclinetLog(LOG_INFO,"%s %d set ip_type=%s",__func__,__LINE__,miner_cfg.ip_type);
			bw_miner_set_network(0);
		}else if (!strcmp(topic_name,TOPIC_SET_POOL)){
			bw_miner_set_pools();
		}else if (!strcmp(topic_name,TOPIC_SET_POSITION)){
			bw_miner_set_position();
		}else if (!strcmp(topic_name,TOPIC_SET_REBOOT)){
			bw_miner_set_reboot();
		}else if (!strcmp(topic_name,TOPIC_SET_NUMBER)){
			bw_miner_set_number();
		}else if (!strcmp(topic_name,TOPIC_SET_UPDATE)){
			bw_miner_set_update_flag(true);
		}
	}
	
	//批量设置
	int batchsetflag = 0;
	bw_miner_judge_batch_set_flag(&batchsetflag);
	if (batchsetflag)
	{
		if (!strcmp(topic_name,TOPIC_BATCH_SET_POOLS)){
			bw_miner_set_pools();
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_REBOOT)){
			bw_miner_set_reboot();
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_UPDATE)){
			bw_miner_set_update_flag(true);
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_NETWORK)){
			bw_miner_set_network(1);
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_THRESHOULD)){
			bw_miner_set_threshould();
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_FAN_SPEED)){
			bw_miner_set_fan_speed();
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_PERIODIC_TIME)){
			bw_miner_set_perioidc_time();
		}else if (!strcmp(topic_name,TOPIC_BATCH_SET_FREQUENCY)){
			bw_miner_set_frequency();
		}
	}	
	return 0;	
}


void publish_callback(void** unused, struct mqtt_response_publish *published) 
{
	/* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
	char* topic_name = (char*) malloc(published->topic_name_size + 1);
	char* payload_data = (char*) malloc(published->application_message_size + 1+2);
	memcpy(topic_name, published->topic_name, published->topic_name_size);
	topic_name[published->topic_name_size] = '\0';
	memcpy(payload_data,(const char *)(published->application_message - 2),published->application_message_size+2);// << 2 bytes
	payload_data[published->application_message_size+2] = '\0';

	bwclinetLog(LOG_INFO,"Received publish('%s'): %s size:%d", topic_name, (const char*) payload_data,strlen(payload_data));
	bw_mqtt_miner_set_cfg(topic_name,(const char*) payload_data);
	if (topic_name){
		free(topic_name);
		topic_name = NULL;
	}
	if (payload_data){
		free(payload_data);
		payload_data = NULL;
	}
}


void* client_refresher(void* client)
{
    while(1) 
    {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}

int mqtt_client_subscribe(struct mqtt_client *client)
{
	if (!client)
		return -1;
	
	//subcribe
    mqtt_subscribe(client, TOPIC_SET_NETWORK, 0);
    mqtt_subscribe(client, TOPIC_SET_POOL, 0);
    mqtt_subscribe(client, TOPIC_SET_POSITION, 0);
    mqtt_subscribe(client, TOPIC_SET_REBOOT, 0);
    mqtt_subscribe(client, TOPIC_SET_NUMBER, 0);
    mqtt_subscribe(client, TOPIC_SET_UPDATE, 0);
	
	//batch subcribe
    mqtt_subscribe(client, TOPIC_BATCH_SET_POOLS, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_REBOOT, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_UPDATE, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_NETWORK, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_TEMPERATURE_THRESHOULD, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_MHS_THRESHOULD, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_THRESHOULD, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_FAN_SPEED, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_PERIODIC_TIME, 0);
    mqtt_subscribe(client, TOPIC_BATCH_SET_FREQUENCY, 0);
	
	return 0;
}

int bw_miner_connect_mqtt_server(struct mqtt_client *client,const char *addr,const char *port)
{
	/* open the non-blocking TCP socket (connecting to the broker) */
	int sockfd = mqtt_pal_sockopen(addr, port, AF_INET);
	if (sockfd == -1) {
		perror("Failed to open socket: ");
		bwclinetLog(LOG_ERR,"error: Failed to open socket");
		exit_mqtt_client(EXIT_FAILURE, sockfd, NULL);
	}

	/* setup a client */
	uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
	uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
	char client_id[128]={0};
	unsigned char mac[16];
	char jsonMsg[4096]={0};
	int nRet = -1;
	
	nRet = mqtt_init(client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);

	if(get_local_mac_address("eth0", (char *)mac) < 0)
	{
		bwclinetLog(LOG_ERR,"Failed to get eth0 mac addr.");
		return nRet;
	}
	snprintf(client_id, sizeof(client_id), "%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	bwclinetLog(LOG_INFO,"client_id %s",client_id);

	/*connect to broker*/
	nRet = mqtt_connect(client, client_id, NULL, NULL, 0, minerSysInfo.paas_user, minerSysInfo.paas_pwd, 0, 400);

	/* check that we don't have any errors */
	if (client->error != MQTT_OK) {
		fprintf(stderr, "error: %s\n", mqtt_error_str(client->error));
		bwclinetLog(LOG_ERR,"error: %s", mqtt_error_str(client->error));
		exit_mqtt_client(EXIT_FAILURE, sockfd, NULL);
	}
	
	/* start a thread to refresh the client (handle egress and ingree client traffic) */
	if(pthread_create(&client_daemon, NULL, client_refresher, client)) {
		fprintf(stderr, "Failed to start client daemon.\n");
		bwclinetLog(LOG_ERR,"error: Failed to start client daemon");
		exit_mqtt_client(EXIT_FAILURE, sockfd, NULL);
	}
	
	bw_mqtt_miners_join_msg(jsonMsg);
	
	/* publish the json msg */
	nRet = mqtt_publish(client, TOPIC_MINERS_JOIN, jsonMsg, strlen(jsonMsg), MQTT_PUBLISH_QOS_1);
	
	printf("%s %s\n\n\n",__func__,jsonMsg);
	bwclinetLog(LOG_INFO,"%s type:%s ip:%s mac:%s stauts:%s version:%s ip_type:%s netmask:%s gateway:%s"
							"dns:%s periodic time:%d pool1_status:%s pool1_diff:%d pool1_addr:%s pool1_miner_addr:%s"
							"pool1_pwd:%s pool2_stauts:%s pool2_diff:%d pool2_addr:%s pool2_miner_addr:%s pool2_pwd:%s"
							"pool3_stauts:%s pool3_diff:%d pool3_addr:%s pool3_miner_addr:%s pool3_pwd:%s number:%s postion:%s",
							__func__,minerSysInfo.type,minerSysInfo.ip,minerSysInfo.mac,minerSysInfo.status,\
							minerSysInfo.version,minerSysInfo.ip_type,minerSysInfo.netmask,minerSysInfo.gateway,\
							minerSysInfo.dns,minerSysInfo.periodic_time,minerSysInfo.pool1_status,minerSysInfo.pool1_diff,\
							minerSysInfo.pool1_addr,minerSysInfo.pool1_miner_addr,minerSysInfo.pool1_password,\
							minerSysInfo.pool2_status,minerSysInfo.pool2_diff,minerSysInfo.pool2_addr,minerSysInfo.pool2_miner_addr,\
							minerSysInfo.pool2_password,minerSysInfo.pool3_status,minerSysInfo.pool3_diff,minerSysInfo.pool3_addr,\
							minerSysInfo.pool3_miner_addr,minerSysInfo.pool3_password,minerSysInfo.number,minerSysInfo.position);

	/* check for errors */
	if (client->error != MQTT_OK) {
	  fprintf(stderr, "error: %s\n", mqtt_error_str(client->error));
	  bwclinetLog(LOG_ERR,"error: %s", mqtt_error_str(client->error));
	  exit_mqtt_client(EXIT_FAILURE, sockfd, &client_daemon);
	}
	
	return nRet;
}
void *bw_miner_sys_info_detect_thread(void *args)
{
	if (!args){
		perror("miner sys info detect thread params invailed.\n");
		return NULL;
	}
	pthread_detach(pthread_self());

	char jsonMsg[4096]={0};
	int dev_flag=0;
	int sem_flag = 0;
	int changed_cnt = 0;
	minerSysInfo.msg = (char *)jsonMsg;
	while(!pquit) {
		memset(minerSysInfo.msg,0,strlen(jsonMsg));
		bw_miner_sys_info_publish_msg(minerSysInfo.msg,&changed_cnt);

		RUNNING_INFO_STAT2(minerRuningInfo.status,DEV_STATUS2, dev_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.status,DEV_STATUS3, dev_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.status,DEV_STATUS4, dev_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.status,DEV_STATUS5, dev_flag, sem_flag)

		if (sem_flag || changed_cnt){
			bw_semSignal(&minerSysInfo.sysinfo_sem);
			sem_flag = 0;
			changed_cnt = 0;
		}
		usleep(2000000UL);
	}
	return NULL;
}

void *bw_miner_sys_info_publish_thread(void *args)
{
	if (!args){
		perror("miners thread params invailed.\n");
		return NULL;
	}
	struct mqtt_client *client = (struct mqtt_client *)args;
	pthread_detach(pthread_self());

	char topic[128] = {0};
	unsigned long periodicTime = 0UL;
	while(!pquit) {
		
		//常规消息按设定的周期上报
		periodicTime = minerSysInfo.periodic_time;//min
		int status = bw_semWait(&minerSysInfo.sysinfo_sem, periodicTime*60);
		if(status!=0)
			break;

		if (!strcmp(minerRuningInfo.status,DEV_STATUS2)||
			!strcmp(minerRuningInfo.status,DEV_STATUS3)||
			!strcmp(minerRuningInfo.status,DEV_STATUS4)||
			!strcmp(minerRuningInfo.status,DEV_STATUS5))
			sprintf(topic,"%s",TOPIC_MINERS_ALERT);
		else if (!strcmp(minerRuningInfo.status,DEV_STATUS0))
			sprintf(topic,"%s",TOPIC_MINERS_NORMAL);

		/* publish the changed info */
		if (minerSysInfo.msg && strcmp(minerSysInfo.msg,"")){
			mqtt_publish(client, topic, minerSysInfo.msg, strlen(minerSysInfo.msg), MQTT_PUBLISH_QOS_1);
			printf("%s %s\n\n\n",__func__,minerSysInfo.msg);
			bwclinetLog(LOG_INFO,"%s type:%s ip:%s mac:%s stauts:%s version:%s ip_type:%s netmask:%s gateway:%s"
									"dns:%s periodic time:%d pool1_status:%s pool1_diff:%d pool1_addr:%s pool1_miner_addr:%s"
									"pool1_pwd:%s pool2_stauts:%s pool2_diff:%d pool2_addr:%s pool2_miner_addr:%s pool2_pwd:%s"
									"pool3_stauts:%s pool3_diff:%d pool3_addr:%s pool3_miner_addr:%s pool3_pwd:%s number:%s postion:%s",
									__func__,minerSysInfo.type,minerSysInfo.ip,minerSysInfo.mac,minerSysInfo.status,\
									minerSysInfo.version,minerSysInfo.ip_type,minerSysInfo.netmask,minerSysInfo.gateway,\
									minerSysInfo.dns,minerSysInfo.periodic_time,minerSysInfo.pool1_status,minerSysInfo.pool1_diff,\
									minerSysInfo.pool1_addr,minerSysInfo.pool1_miner_addr,minerSysInfo.pool1_password,\
									minerSysInfo.pool2_status,minerSysInfo.pool2_diff,minerSysInfo.pool2_addr,minerSysInfo.pool2_miner_addr,\
									minerSysInfo.pool2_password,minerSysInfo.pool3_status,minerSysInfo.pool3_diff,minerSysInfo.pool3_addr,\
									minerSysInfo.pool3_miner_addr,minerSysInfo.pool3_password,minerSysInfo.number,minerSysInfo.position);
		}

		/* check for errors */
		if (client->error != MQTT_OK) {
		  fprintf(stderr, "error: %s\n", mqtt_error_str(client->error));
		  bwclinetLog(LOG_ERR,"error: %s", mqtt_error_str(client->error));
		  exit_mqtt_client(EXIT_FAILURE, client->socketfd, &client_daemon);
		}
	}   
	return NULL;
}
void *bw_miner_runing_info_detect_thread(void *args)
{
	if (!args){
		perror("miner runing inf detect thread params invailed.\n");
		return NULL;
	}
	pthread_detach(pthread_self());

	char jsonMsg[4096]={0};
	int temp_flag=0,mhs_flag=0,fan1_flag=0,fan2_flag=0;
	int sem_flag = 0;
	minerRuningInfo.msg = (char *)jsonMsg;
	while(!pquit) {
		memset(minerRuningInfo.msg,0,strlen(jsonMsg));
		bw_miner_runing_info_publish_msg(minerRuningInfo.msg);

		RUNNING_INFO_STAT1(minerRuningInfo.temperature,minerRuningInfo.temperature_threshould, temp_flag, sem_flag)
		RUNNING_INFO_STAT1(minerRuningInfo.mhs,minerRuningInfo.mhs_threshould, mhs_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.fan1_status,FANS_STAUTS1, fan1_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.fan1_status,FANS_STAUTS2, fan1_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.fan2_status,FANS_STAUTS1, fan2_flag, sem_flag)
		RUNNING_INFO_STAT2(minerRuningInfo.fan2_status,FANS_STAUTS2, fan2_flag, sem_flag)

		if (sem_flag){
			bw_semSignal(&minerRuningInfo.runinginfo_sem);
			sem_flag = 0;
		}
		usleep(2000000UL);
	}
	return NULL;
}

void *bw_miner_runing_info_publish_thread(void *args)
{
	if (!args){
		perror("miner runing info publish thread params invailed.\n");
		return NULL;
	}
	struct mqtt_client *client = (struct mqtt_client *)args;
	pthread_detach(pthread_self());

	char topic[128] = {0};
	unsigned long periodicTime = 0UL;
	while(!pquit) {
		//常规消息按设定的周期上报
		periodicTime = minerSysInfo.periodic_time;//min
        int status = bw_semWait(&minerRuningInfo.runinginfo_sem, periodicTime*60);
        if(status!=0)
            break;

		if (minerRuningInfo.temperature >= minerRuningInfo.temperature_threshould
			|| minerRuningInfo.mhs_threshould >= minerRuningInfo.mhs
			|| (!strcmp(minerRuningInfo.fan1_status,FANS_STAUTS1))
			|| (!strcmp(minerRuningInfo.fan1_status,FANS_STAUTS2))
			|| (!strcmp(minerRuningInfo.fan2_status,FANS_STAUTS1))
			|| (!strcmp(minerRuningInfo.fan2_status,FANS_STAUTS2)))
			sprintf(topic,"%s",TOPIC_MINER_ALERT);
		else
			sprintf(topic,"%s",TOPIC_MINER_NORMAL);
		
		/* publish the runing info */
		if (minerRuningInfo.msg && strcmp(minerRuningInfo.msg,"")){
			mqtt_publish(client, topic, minerRuningInfo.msg, strlen(minerRuningInfo.msg), MQTT_PUBLISH_QOS_1);
			printf("%s %s\n\n\n",__func__,minerRuningInfo.msg);
			bwclinetLog(LOG_INFO,"%s status:%s ip:%s mac:%s mhs:%d avg_mhs:%d accept:%d reject:%d diff:%d boards:%d"
										"board1_mhs:%d board1_avg_mhs:%d board1_temp:%d board1_freq:%d board1_accept:%d board1_reject:%d"
										"board2_mhs:%d board2_avg_mhs:%d board2_temp:%d board2_freq:%d board2_accept:%d board2_reject:%d"
										"board3_mhs:%d board3_avg_mhs:%d board3_temp:%d board3_freq:%d board3_accept:%d board3_reject:%d"
										"board4_mhs:%d board4_avg_mhs:%d board4_temp:%d board4_freq:%d board4_accept:%d board4_reject:%d"
										"fans:%d fan1_stauts:%s fan1_speed:%d,fan2_stauts:%s fan2_speed:%d duration:%lu temp_threshould:%d mhs_threshould:%d",
										__func__,minerRuningInfo.status,minerRuningInfo.ip,minerRuningInfo.mac,minerRuningInfo.mhs,
										minerRuningInfo.avg_mhs,minerRuningInfo.accept,minerRuningInfo.reject,minerRuningInfo.diff,
										minerRuningInfo.boards,minerRuningInfo.board1_mhs,minerRuningInfo.board1_avg_mhs,minerRuningInfo.board1_temperate,
										minerRuningInfo.board1_frequency,minerRuningInfo.board1_accept,minerRuningInfo.board1_reject,
										minerRuningInfo.board2_mhs,minerRuningInfo.board2_avg_mhs,minerRuningInfo.board2_temperate,
										minerRuningInfo.board2_frequency,minerRuningInfo.board2_accept,minerRuningInfo.board2_reject,
										minerRuningInfo.board3_mhs,minerRuningInfo.board3_avg_mhs,minerRuningInfo.board3_temperate,
										minerRuningInfo.board3_frequency,minerRuningInfo.board3_accept,minerRuningInfo.board3_reject,
										minerRuningInfo.board4_mhs,minerRuningInfo.board4_avg_mhs,minerRuningInfo.board4_temperate,
										minerRuningInfo.board4_frequency,minerRuningInfo.board4_accept,minerRuningInfo.board4_reject,
										minerRuningInfo.fans,minerRuningInfo.fan1_status,minerRuningInfo.fan1_speed,minerRuningInfo.fan2_status,
										minerRuningInfo.fan2_speed,minerRuningInfo.duration,minerRuningInfo.temperature_threshould,minerRuningInfo.mhs_threshould);

		}
		
		/* check for errors */
		if (client->error != MQTT_OK) {
		  fprintf(stderr, "error: %s\n", mqtt_error_str(client->error));
		  bwclinetLog(LOG_ERR,"error: %s", mqtt_error_str(client->error));
		  exit_mqtt_client(EXIT_FAILURE, client->socketfd, &client_daemon);
		}
	}	
	return NULL;
}

void *bw_miner_report_publish_thread(void *args)
{
	if (!args){
		perror("miner thread params invailed.\n");
		return NULL;
	}
	struct mqtt_client *client = (struct mqtt_client *)args;
	pthread_detach(pthread_self());

	while(!quit) 
	{
		if (bw_miner_get_update_flag() == true)
		{
			bw_miner_update(client);
			bw_miner_set_update_flag(false);
		}
	}	
	return NULL;
}

int bw_miner_init_sysinfo()
{
	memset(&minerSysInfo,0,sizeof(minerSysInfo));
	memset(&preminerSysInfo,0,sizeof(preminerSysInfo));
	pthread_mutex_init(&minerSysInfo.sysinfo_lock, NULL);
	pthread_mutex_init(&preminerSysInfo.sysinfo_lock, NULL);
    int status = bw_semCreate(&minerSysInfo.sysinfo_sem, 1, 0);
    assert(status==0);
	return 0;
}
int bw_miner_uninit_sysinfo()
{
	memset(&minerSysInfo,0,sizeof(minerSysInfo));
	memset(&preminerSysInfo,0,sizeof(preminerSysInfo));
	pthread_mutex_destroy(&minerSysInfo.sysinfo_lock);
	pthread_mutex_destroy(&preminerSysInfo.sysinfo_lock);
    bw_semDelete(&minerSysInfo.sysinfo_sem);
	return 0;
}
int bw_miner_init_runinginfo()
{
	memset(&minerRuningInfo,0,sizeof(minerRuningInfo));
	memset(&preminerRuningInfo,0,sizeof(preminerRuningInfo));
	pthread_mutex_init(&minerRuningInfo.runinginfo_lock, NULL);
	pthread_mutex_init(&preminerRuningInfo.runinginfo_lock, NULL);
    int status = bw_semCreate(&minerRuningInfo.runinginfo_sem, 1, 0);
    assert(status==0);
	return 0;
}
int bw_miner_uninit_runinginfo()
{
	memset(&minerRuningInfo,0,sizeof(minerRuningInfo));
	memset(&preminerRuningInfo,0,sizeof(preminerRuningInfo));
	pthread_mutex_destroy(&minerRuningInfo.runinginfo_lock);
	pthread_mutex_destroy(&preminerRuningInfo.runinginfo_lock);
    bw_semDelete(&minerRuningInfo.runinginfo_sem);
	return 0;
}
int bw_miner_init_cfginfo()
{
	memset(&miner_cfg,0,sizeof(miner_cfg));
	pthread_mutex_init(&miner_cfg.cfginfo_lock, NULL);
	return 0;
}
int bw_miner_uninit_cfginfo()
{
	memset(&miner_cfg,0,sizeof(miner_cfg));
	pthread_mutex_destroy(&miner_cfg.cfginfo_lock);
	return 0;
}

int main(int argc, const char *argv[]) 
{
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);

	bw_miner_init_sysinfo();
	bw_miner_init_runinginfo();
	bw_miner_init_cfginfo();

	//connect mqtt server
	struct mqtt_client client;
	char pass_port[8]={0};
	static int trytimes = 0;
	static int start = 1;
	while(!quit && start)
	{
		while(!quit)
		{
			bw_mqtt_miner_get_summary_info();
			
			printf("====> Mqtt client version %s\n",MQTT_CLIENT_VERSION);
			printf("====> Mqtt server enable paas %d ip %s port %d\n",minerSysInfo.enable_paas,minerSysInfo.paas_ip,minerSysInfo.paas_port);
			bwclinetLog(LOG_INFO,"====> Mqtt client version %s",MQTT_CLIENT_VERSION);
			bwclinetLog(LOG_INFO,"====> Mqtt server enable paas %d ip %s port %d",minerSysInfo.enable_paas,minerSysInfo.paas_ip,minerSysInfo.paas_port);
			
			sprintf(pass_port,"%d",minerSysInfo.paas_port);
		
			if (minerSysInfo.enable_paas == true && 0 != strcmp(minerSysInfo.paas_ip,"") &&  1883 == minerSysInfo.paas_port){
				strncpy(preminerSysInfo.paas_ip,minerSysInfo.paas_ip,sizeof(minerSysInfo.paas_ip));
				preminerSysInfo.paas_port = minerSysInfo.paas_port;
				preminerSysInfo.enable_paas = minerSysInfo.enable_paas;
				if (bw_miner_connect_mqtt_server(&client,minerSysInfo.paas_ip,pass_port) != MQTT_OK){
					bwclinetLog(LOG_ERR,"====> connnect mqtt server failed,pls check server is power on or not or server ip cfg.");
					printf("====> connnect mqtt server failed,pls check server is power on or not or server ip cfg.");
				}else{
					start = 0;
					printf("====> connect mqtt server sucess.\n");
					break;
				}
			}else{
				bwclinetLog(LOG_ERR,"paas not enable or paas ip and port cfg err,pls check it.");
				printf("====> paas not enable or paas ip and port cfg err,pls check it.");
			}
			if (++trytimes >= 5){
				usleep(1*60*1000000UL);//1min
				trytimes = 0;
			}else{
				usleep(3000000UL);//3s
			}
		}
		
		pthread_t miners_ptd,miner_ptd,miners_detect_ptd,miner_detect_ptd,miner_report_ptd;
		//miner系统信息检测
		if(pthread_create(&miners_detect_ptd, NULL, bw_miner_sys_info_detect_thread,&client) != 0)
		{
			bwclinetLog(LOG_ERR,"%s create miner sys info detect thread failed.",__func__);
			return -1;
		}
		
		//miner系统信息上报线程
		if(pthread_create(&miners_ptd, NULL, bw_miner_sys_info_publish_thread,&client) != 0)
		{
			bwclinetLog(LOG_ERR,"%s create miner sys info publish thread failed.",__func__);
			return -1;
		}

		//miner运行状态检测
		if(pthread_create(&miner_detect_ptd, NULL, bw_miner_runing_info_detect_thread,&client) != 0)
		{
			bwclinetLog(LOG_ERR,"%s create miner runing info detect thread failed.",__func__);
			return -1;
		}

		//miner运行状态上报线程
		if(pthread_create(&miner_ptd, NULL, bw_miner_runing_info_publish_thread,&client) != 0)
		{
			bwclinetLog(LOG_ERR,"%s create miner runing info publish thread failed.",__func__);
			return -1;
		}

		//miner订阅消息
		mqtt_client_subscribe(&client);

		//miner批量控制及设置状态上报线程
		if(pthread_create(&miner_report_ptd, NULL, bw_miner_report_publish_thread,&client) != 0)
		{
			bwclinetLog(LOG_ERR,"%s create miner report publish thread failed.",__func__);
			return -1;
		}
		
	  	while(!quit)
		{
			if (preminerSysInfo.enable_paas != minerSysInfo.enable_paas || 0 != strcmp(preminerSysInfo.paas_ip,minerSysInfo.paas_ip) || preminerSysInfo.paas_port != minerSysInfo.paas_port){
				bwclinetLog(LOG_INFO,"%s modify miner paas cfg , reconnect paas!",__func__);
				pquit = 1;
				start = 1;
				bw_semSignal(&minerSysInfo.sysinfo_sem);
				bw_semSignal(&minerRuningInfo.runinginfo_sem);
				usleep(10000000UL);
				break;
			}
			usleep(3000000UL);
		}
	}
    /* disconnect */
    bwclinetLog(LOG_INFO,"disconnecting from %s %d", minerSysInfo.paas_ip,minerSysInfo.paas_port);
    printf("disconnecting from %s %d\n", minerSysInfo.paas_ip,minerSysInfo.paas_port);

	return 0;
}

void exit_mqtt_client(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(*client_daemon);
	bw_miner_uninit_sysinfo();
	bw_miner_uninit_runinginfo();
	bw_miner_uninit_cfginfo();
	bwclinetLog(LOG_ERR,"exit_mqtt_client...");
    exit(status);
}
