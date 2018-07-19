#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__


#define MQTT_CLIENT_VERSION "Mqtt_client_v1.1"
#define BUFSIZE 			1500
#define UPDATE_FILE_PATH	"/tmp/miner_tool_update.tar.gz"
#define OSA_TIMEOUT_NONE        ((int) 0)  // no timeout
#define OSA_TIMEOUT_FOREVER     ((int)-1)  // wait forever

#define RUNNING_INFO_STAT1(A,B,F1,F2)\
	if(A >= B && F1 == 0){F1=1;F2=1;}\
	else if(A < B && F1 == 1){F1=0;F2=1;}\

#define RUNNING_INFO_STAT2(A,B,F1,F2)\
	if(!strcmp(A,B) && F1 == 0){F1=1;F2=1;}\
	else if(strcmp(A,B) && F1 == 1){F1=0;F2=1;}\

#define L21 "L21"
#define L61	"L61"

//dev status
#define DEV_STATUS0							"running"
#define DEV_STATUS1							"offline"
#define DEV_STATUS2							"error"
#define DEV_STATUS3							"low mhs"
#define DEV_STATUS4							"zero mhs"
#define DEV_STATUS5							"ip conflict"

//pools status
#define POOLS_STAUTS0						"connected"
#define POOLS_STAUTS1						"disconnected"
#define POOLS_STAUTS2						"error"

//fans status
#define FANS_STAUTS0						"running"
#define FANS_STAUTS1						"stopped"
#define FANS_STAUTS2						"error"

//publish msg
#define TOPIC_MINERS_JOIN					"join"
#define TOPIC_MINERS_NORMAL					"miners normal publish"
#define TOPIC_MINERS_PERIODIC				"miners periodic publish"
#define TOPIC_MINERS_ALERT					"miners alert"
#define TOPIC_MINER_NORMAL					"miner normal publish"
#define TOPIC_MINER_PERIODIC				"miner periodic publish"
#define TOPIC_MINER_ALERT					"miner alert"
#define TOPIC_UPDATE_PROGRESS				"update_progress"
#define TOPIC_UPDATE_STATUS					"update_status"

//subscribe msg
#define TOPIC_SET_NETWORK 					"set_network"
#define TOPIC_SET_POOL 						"set_pools"
#define TOPIC_SET_FREQUENCY 				"set_frequency"
#define TOPIC_SET_PERIODIC_TIME 			"set_periodic_time"
#define TOPIC_SET_POSITION 					"set_position"
#define TOPIC_SET_FAN_SPEED 				"set_fan_speed"
#define TOPIC_SET_TEMPERATURE_THRESHOULD 	"set_temperature_threshould"
#define TOPIC_SET_MHS_THRESHOULD 			"set_mhs_threshould"
#define TOPIC_SET_REBOOT					"set_reboot"
#define TOPIC_SET_NUMBER					"set_number"
#define TOPIC_SET_UPDATE					"set_update"

//batch subscribe msg
#define TOPIC_BATCH_SET_POOLS					"batch_set_pools"
#define TOPIC_BATCH_SET_REBOOT					"batch_set_reboot"
#define TOPIC_BATCH_SET_UPDATE					"batch_set_update"
#define TOPIC_BATCH_SET_NETWORK 				"batch_set_network"
#define TOPIC_BATCH_SET_TEMPERATURE_THRESHOULD	"batch_set_temperature_threshould"
#define TOPIC_BATCH_SET_MHS_THRESHOULD			"batch_set_mhs_threshould"
#define TOPIC_BATCH_SET_THRESHOULD				"batch_set_threshould"
#define TOPIC_BATCH_SET_FAN_SPEED 				"batch_set_fan_speed"
#define TOPIC_BATCH_SET_PERIODIC_TIME 			"batch_set_periodic_time"
#define TOPIC_BATCH_SET_FREQUENCY 				"batch_set_frequency"


typedef struct {

  int count;
  int maxCount;
  pthread_mutex_t lock;
  pthread_cond_t  cond;
} bw_SemHndl;

enum MINER_CFG_SET
{
	SET_POOL1=0,
	SET_POOL2,
	SET_POOL3,
	SET_USE_FREQUENCY_ALL,
	SET_FREQUENCY_ALL,
	SET_FREQUENCY1,
	SET_FREQUENCY2,
	SET_FREQUENCY3,
	SET_FREQUENCY4,
	SET_PERIODIC_TIME,
	SET_POSITION,
	SET_FAN1_SPEED,
	SET_FAN2_SPEED,
	SET_TEMPERATURE_THRESHOULD,
	SET_MHS_THRESHOULD,
	SET_NUMBER,
};

typedef struct bw_miner_sysinfo_t
{
	int id;
	char date[32];
	char type[32];
	char ip[16];
	char mac[32];
	char status[16];
	char version[64];
	char ip_type[16];
	char netmask[16];
	char gateway[16];
	char dns[16];
	int periodic_time;
	char pool1_status[16];
	int pool1_diff;
	char pool1_addr[64];
	char pool1_miner_addr[64];
	char pool1_password[32];
	char pool2_status[16];
	int pool2_diff;
	char pool2_addr[64];
	char pool2_miner_addr[64];
	char pool2_password[32];
	char pool3_status[16];
	int pool3_diff;
	char pool3_addr[64];
	char pool3_miner_addr[64];
	char pool3_password[32];
	char number[16];
	char position[128];
	bool enable_paas;
	char paas_ip[32];
	int  paas_port;
	char paas_user[32];
	char paas_pwd[32];
	char *msg;
	pthread_mutex_t sysinfo_lock;
	bw_SemHndl sysinfo_sem;
}bw_miner_sysinfo;

typedef struct bw_miner_runinginfo_t
{
	int id;
	char status[16];
	char date[32];
	char ip[16];
	char mac[32];
	int mhs;
	int avg_mhs;
	int accept;
	int reject;
	int diff;
	int temperature;
	int boards;
	int board1_mhs;
	int board1_avg_mhs;
	int board1_temperate;
	int board1_frequency;
	int board1_accept;
	int board1_reject;
	int board2_mhs;
	int board2_avg_mhs;
	int board2_temperate;
	int board2_frequency;
	int board2_accept;
	int board2_reject;
	int board3_mhs;
	int board3_avg_mhs;
	int board3_temperate;
	int board3_frequency;
	int board3_accept;
	int board3_reject;
	int board4_mhs;
	int board4_avg_mhs;
	int board4_temperate;
	int board4_frequency;
	int board4_accept;
	int board4_reject;
	int fans;
	char fan1_status[16];
	int fan1_speed;
	char fan2_status[16];
	int fan2_speed;
	unsigned long duration;
	int temperature_threshould;
	int mhs_threshould;
	char *msg;
	pthread_mutex_t runinginfo_lock;
	bw_SemHndl runinginfo_sem;
}bw_miner_runinginfo;

typedef struct bw_miner_cfginfo_t
{
	int match;
	char type[32];
	char ip[16];
	char mac[16];
	char ip_type[16];
	char netmask[16];
	char gateway[16];
	char dns[16];
	int periodic_time;
	char pool1_addr[64];
	char pool1_miner_addr[64];
	char pool1_password[32];
	char pool2_addr[64];
	char pool2_miner_addr[64];
	char pool2_password[32];
	char pool3_addr[64];
	char pool3_miner_addr[64];
	char pool3_password[32];
	char position[64];
	char number[16];
	char update_url[128];
	char ipAarry[256][64];
	int  mhs_threshould;
	int  temp_threshould;
	int  fan_speed[2];
	bool use_frequency_all;
	int  frequency_all;
	int  frequency_1;
	int  frequency_2;
	int  frequency_3;
	int  frequency_4;
	bool update_report_flag;
	pthread_mutex_t cfginfo_lock;
}bw_miner_cfginfo;

typedef struct option_t {
	const char *name;
	int (*func)(const char *,int);
}option;

typedef struct myprogress_t {
  curl_off_t resumeByte;
  time_t lastTime;
  double downloadFileLength;
  CURL *curl;
  struct mqtt_client *client;
}myprogress;


// sem
int bw_semCreate(bw_SemHndl *hndl, int maxCount, int initVal);
int bw_semWait(bw_SemHndl *hndl, int timeout);
int bw_semSignal(bw_SemHndl *hndl);
int bw_semDelete(bw_SemHndl *hndl);

enum {
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG,
};
	
void _bwclientLog(int prio, const char *str, bool force)
{
		char datetime[64];
		struct timeval tv = {0, 0};
		struct tm *tm;

		gettimeofday(&tv, NULL);

		const time_t tmp_time = tv.tv_sec;
		tm = localtime(&tmp_time);

		snprintf(datetime, sizeof(datetime), " [%02d:%02d:%02d] ",
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec);

		/* Only output to stderr if it's not going to the screen as well */
	//	if (!isatty(fileno((FILE *)stderr))) 
		{
			//fprintf(stderr, "%s%s\n", datetime, str);	/* atomic write to stderr */
			//fflush(stderr);
		}

		//FILE *stream = NULL;   
		//char cmdBuf[4096]={0};
		char dbg_buf[3092]={0};
		sprintf(dbg_buf,"%s%s",datetime,str);
		//在bw6000上报错broken pipe暂屏蔽(L21 OK)
		//sprintf(cmdBuf,"echo -n '{\"command\":\"logout\",\"parameter\":\"%s\"}' | nc localhost 4028",dbg_buf);
		//printf("%s  %s\n",__func__,cmdBuf);
		//stream = popen(cmdBuf, "r");
		//pclose(stream);
}

#define bwclinetLog(prio, fmt, ...) do { \
				if (prio != LOG_DEBUG)\
				{ \
						char tmp42[2048]; \
						snprintf(tmp42, sizeof(tmp42), fmt, ##__VA_ARGS__); \
						_bwclientLog(prio, tmp42, false); \
				} \
			} while (0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define sleep(secs) Sleep((secs) * 1000)

#ifdef _MSC_VER
#define snprintf(...) _snprintf(__VA_ARGS__)
#define strdup(...) _strdup(__VA_ARGS__)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define strcasecmp(x,y) _stricmp(x,y)
#define __func__ __FUNCTION__
#define __thread __declspec(thread)
#define _ALIGN(x) __declspec(align(x))
typedef int ssize_t;
#endif

#ifndef _MSC_VER
#define _ALIGN(x) __attribute__ ((aligned(x)))
#endif
#endif