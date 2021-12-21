#include <pthread.h>
#include <curl/curl.h>
#include "gfsconf.h"
#include <sys/resource.h>
#include <sys/types.h>
#include <dirent.h>

#define RELEASE "0.9debug"
#define SOFTWAREEXPIRES 1734392362 // Monday, December 16, 2024 11:39:22 PM 
#define NEXFS_STRUCTVERSION "1.02"

#define FUSE_USE_VERSION 61
#define USESYSLOG 1
#define DEBUGBUILD 1

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
#define STATUSINSHUTDOWN 11 

#define BUFFERSTATENONE 0
#define BUFFERSTATEFD 1
#define BUFFERSTATEMEM 2
#define BUFFERSTATEINASSIGMENT 3
#define BUFFERSTATEINReASSIGMENT 4
#define BUFFERSTATECHANGEFDTOFDMEM 5
#define BUFFERSTATEWAITINGDATALOAD 6


char statustext[12][20];
pid_t MPID;
struct rlimit fdlimit;
long openfds;
long maxopenfds;
long maxusedopenfh;
int configinodeidx;
pthread_mutex_t maxusedopenfhlock;
pthread_mutex_t configinodelock;
pthread_mutex_t openfdslock;
CURLSH *curlshare;
char *MYNAME;
char *NEXFSMOUNTPOINT;
time_t BINARYEXPIRES;
int nexfsreleasetype;
int nexfsreadonly;
int nexfsexpired;
int nexfsinshutdown;
DIR *T1SDS;
DIR *T2SDS;
int64_t T1SDIRFD;
int64_t T2SDIRFD;
uint64_t managedcapacity;
long managedcapacityfd;
long managedcapacityfdrep;
pthread_mutex_t managedcapacitylock; 
short wsplice;
int activewritereqs;
pthread_mutex_t activewritereqslock; 

struct struct_gfsfilehandlepages
{
  long fdd;
  pthread_mutex_t mlock;
  int lock;
  int part;
  int nextpart;
  int state;
  int bufferdirty;
  int datafileupdated;
  clock_t lastused; // oldest is reused first
  char *page;
  long pagesize;
  long pagedatasize;
  long loadedpagesize; 
  CURL *curl_handle;
  long t3size;
  char t3etag[37];
  int t3compressed;
  int t3encrypted;
  char localmd5hash[37];
  int opendatafileactivethreadcount;
  int opendatafileexcluselock;
  pthread_mutex_t odelock;
  pthread_mutex_t odclock;
};

struct struct_gfsopenfilehandles
{
  int64_t fds;
  int64_t fds2;
  ino_t inode;
  pthread_mutex_t lock;
  int openfhcount;
  int lockowner;
  int previouspart; // used for stats and tiering
  int allocatedpages;    // number of allocated cache pages currently in this handle
  int configrequest;
  int cmdrequest;
  off_t jobinfo;
  char gfsid[37];  // holds live config var name in case of a configrequest
  int lockedtotier;  // the file is locked to a certain tier
  int mintiered;  // the lowest tier that contains datafiles
  int maxtiered;  // the highest tier that contains datafiles
  int datapartsize;  // the files data part size (chunksize)
  int updated;
  long allocatedchunks; // the number of written chunks
  long allocatedchunkslast; // the number of written chunks
  time_t openfilesmartprotect;
  time_t openfilesmarttier;
  struct struct_gfsfilehandlepages **filehandlepages;  // a pointer to an array of pages, allocated as char*MAXHANDLEPAGES;
  time_t oldesttier1datafile;
  time_t oldesttier2datafile;
};

struct struct_gfsfh 
{
  int64_t ofh;
  int READABLE;
  int WRITABLE;
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
