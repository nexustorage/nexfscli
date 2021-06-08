#include <syslog.h>
#include <inttypes.h>

int gfs_opensyslog(const char *);
void gfs_printsyslogfacilities(char *, int);
int gfs_tosyslog(int, const char * );
int gfs_closesyslog(void);
int gfs_setloglevel(char *, int);
void gfs_printloglevels(char *, int);
int gfs_changesyslogfacility();

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
