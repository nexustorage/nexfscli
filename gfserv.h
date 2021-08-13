#include <pthread.h>
#include <curl/curl.h>
#include "gfsconf.h"
#include <sys/resource.h>
#include <sys/types.h>
#include <dirent.h>

#define RELEASE "p0.1.0p3"
#define SOFTWAREEXPIRES 1633089600 // Friday, October 1, 2021 12:00:00 PM GMT
#define NEXFS_STRUCTVERSION "1.01"

#define FUSE_USE_VERSION 61
#define USESYSLOG 1

// #define AWSAccessKeyId "minioadmin"
// #define MAXHANDLEPAGES 2
// #define CACHELOCKWAIT 10

#define RELEASETYPEDEV 0
#define RELEASETYPEBETA 1
#define RELEASETYPEPREVIEW 2
#define RELEASETYPEGA 3

#define NEXFSRELEASETYPE RELEASETYPEDEV 


#ifndef ENOATTR
#if defined(ENODATA)
#define ENOATTR ENODATA
#else
#define ENOATTR ENOENT
#endif
#endif

#define STATUSUNKNOWN 0
#define STATUSOK 1
#define STATUSWARN 2
#define STATUSERR 3
#define STATUSDISABLED 4
#define STATUSENABLED 5
#define STATUSPAUSED 6
#define STATUSINITALISING 7
#define STATUSLOWWATERMARK 8
#define STATUSHIGHWATERMARK 9
#define STATUSFLOORWATERMARK 10 

#define BUFFERSTATENONE 0
#define BUFFERSTATEFD 1
#define BUFFERSTATEMEM 2
#define BUFFERSTATEINASSIGMENT 3
#define BUFFERSTATEINReASSIGMENT 4
#define BUFFERSTATECHANGEFDTOFDMEM 5
#define BUFFERSTATEWAITINGDATALOAD 6

char statustext[11][20];
pid_t MPID;
struct rlimit fdlimit;
long openfds;
long maxopenfds;
pthread_mutex_t openfdslock;
CURLSH *curlshare;
char *MYNAME;
char *NEXFSMOUNTPOINT;
time_t BINARYEXPIRES;
int nexfsreleasetype;
int nexfsreadonly;
DIR *T1SDS;
DIR *T2SDS;
int64_t T1SDIRFD;
int64_t T2SDIRFD;

struct struct_gfsfilehandlepages
{
  long fdd;
  pthread_mutex_t mlock;
  int lock;
  int part;
  int nextpart;
  int state;
  clock_t lastused; // oldest is reused first
  char *page;
  long pagesize;
  long pagedatasize; 
  CURL *curl_handle;
  long t3size;
  char t3etag[37];
  char localmd5hash[37];
  int datafileupdated;
  int opendatafileactivethreadcount;
  int opendatafileexcluselock;
  pthread_mutex_t odelock;
  pthread_mutex_t odclock;
};

struct struct_gfsopenfilehandles
{
  int64_t fds;
  int64_t fds2;
  pthread_mutex_t lock;
  int lockowner;
  int previouspart; // used for stats and tiering
  int allocatedpages;    // number of allocated cache pages currently in this handle
  int configrequest;
  int cmdrequest;
  size_t jobinfo;
  char gfsid[37];  // holds live config var name in case of a configrequest
  int lockedtotier;  // the file is locked to a certain tier
  int mintiered;  // the lowest tier that contains datafiles
  int maxtiered;  // the highest tier that contains datafiles
  int datapartsize;  // the files data part size (chunksize)
  int updated;
  time_t openfilesmartprotect;
  time_t openfilesmarttier;
  struct struct_gfsfilehandlepages **filehandlepages;  // a pointer to an array of pages, allocated as char*MAXHANDLEPAGES;
  time_t oldesttier1datafile;
  time_t oldesttier2datafile;
};

struct struct_serverstatus
{
  int serverstatus;
  int tier1status;
  int tier1thesholdstatus;
  int tier1floorstatus;
  int tier2status;
  int tier2thesholdstatus;
  int tier2floorstatus;
  int tier3status;
  int tier1structstatus;
  int tier2structstatus;
  int jobschedulerstatus;
  int bgmigrationschedulerstatus;
  int deletionschedulerstatus;
  int manualcmdstatus;
};

struct struct_serverstatus *serverstatus;
struct struct_gfsopenfilehandles *gfsfds;
struct fuse_session *se;
struct stat *gfsfds_attrs;
pthread_mutex_t *gfsfds_attrslck;
