#include <syslog.h>
#include <inttypes.h>

int gfs_opensyslog(const char *);
void gfs_printsyslogfacilities(char *, int);
int gfs_tosyslog(int, const char * );
int gfs_closesyslog(void);
int gfs_setloglevel(char *, int);
void gfs_printloglevels(char *, int);
int gfs_changesyslogfacility();
int gfs_log(int, const char *, const char *, ...);


uint64_t syslogcounter;
extern int VALIDDEBUGLEVELS[];
extern char *VALIDDEBUGLEVELSLABELS[];
extern int VALIDSYSLOGFACILITIES[];
extern char *VALIDSYSLOGFACILITY[];

int LOGLEVEL;	

#define LOGCRIT		LOG_CRIT
#define LOGERR		LOG_ERR
#define LOGWARN		LOG_WARNING
#define LOGNOTICE	LOG_NOTICE
#define LOGINFO		LOG_INFO
#define LOGDEBUG	LOG_DEBUG





#define log_warning(fmt, args...)  gfs_log(LOGWARN, "itarget",fmt, ##args);
#define log_error(fmt, args...)  gfs_log(LOGERR, "itarget",fmt, ##args);
#define log_debug(fmt, args...)  gfs_log(LOGDEBUG, "itarget",fmt, ##args);

#define eprintf(fmt, args...)						\
do {									\
	log_error("%s(%d) " fmt, __FUNCTION__, __LINE__, ##args);	\
} while (0)

#define dprintf(fmt, args...)						\
do {									\
		log_debug("%s(%d) " fmt, __FUNCTION__, __LINE__, ##args); \
} while (0)

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
