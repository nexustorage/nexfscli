#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>


#define DEFAULTGFSCONFDIR "/etc/nexfs"
#define GFSCONFIGTAG "2b339ae7a7f04e45960f8a3fcc638869"
#define GFSCMDTAG "2b339ae7a7f04e45960f8a3fcc638870"
#define GFSNULLTAG "2b339ae7a7f04e45960f8a3fcc638800"
#define GFSVALUE 2
#define GFSTAG 1
#define GFSHELP 3
#define GFSVSTRING 4
#define GFSVMIN 5
#define GFSVMAX 6
#define GFSRESTART 7
#define MAXTIERS 3

#define DDIR 1
#define SDIR 2
#define RDIR 3
#define PDIR 4

#define HTTPGET 0
#define HTTPPUT 1
#define HTTPHEAD 2
#define HTTPDELETE 3


int LOWESTTIER;
int demigrationtierorder;

int GFSCONFIGTAGLENGTH;
int GFSCMDTAGLENGTH;
int GFSNULLTAGLENGTH;

// struct stat STATSGFSCONFIGTAG; 
// struct stat STATSGFSCMDTAG; 

// int ATTRBTIMEOUTMS=-1; // needs to be made a configurable

#define GFSCONFIGTAGSCOUNT 72  // tags-1 (release not included)
#define GFSCONFIGTAGSIZE 50

extern char GFSCONFIGTAGS[GFSCONFIGTAGSCOUNT][GFSCONFIGTAGSIZE]; 



// Places to update when adding a configuration value
// 
// gfs_liveconf, gfs_createdefaultconf, gfs_loadconfig, gfs_updateliveconf, GFSCONFIGTAG[][], counters GFSCONFIGTAGSCOUNT & GFSCONFIGTAGSIZE 
//

// Define NEXFSRELEASE
char NEXFSRELEASE[16];  // Nexfs Release
#define NEXFSRLEASETAG "NexFS Release"
#define NEXFSRLEASEHELP "The NexFS Release"

// Define NEXFSPAUSED
int NEXFSPAUSED;  // pause all client read/write data operations 
#define DEFAULTNEXFSPAUSED "0"
#define NEXFSPAUSEDTAG "Nexfs Operations Paused"
#define NEXFSPAUSEDHELP "When paused nexfs will stop accepting file opens/creates/reads and writes, does not effect the job scheduler or bg migrations"
#define NEXFSPAUSEDVALIDATESTRING 0
#define NEXFSPAUSEDVALIDATEMIN 0 
#define NEXFSPAUSEDVALIDATEMAX 1 
#define NEXFSPAUSEDRESTARTREQUIRED 0 

// Define JOBSCHEDULARPAUSED
int JOBSCHEDULARPAUSED;  // if the jobsechedule should run or pause 
#define DEFAULTJOBSCHEDULARPAUSED "0"
#define JOBSCHEDULARPAUSEDTAG "Schedular Paused"
#define JOBSCHEDULARPAUSEDHELP "If the internal jobs schedular is running or paused"
#define JOBSCHEDULARPAUSEDVALIDATESTRING 0
#define JOBSCHEDULARPAUSEDVALIDATEMIN 0 
#define JOBSCHEDULARPAUSEDVALIDATEMAX 1 
#define JOBSCHEDULARPAUSEDRESTARTREQUIRED 0 

// Define BGMIGRATIONSPAUSED 
int BGMIGRATIONSPAUSED;  // if background migrations should run or be paused
#define DEFAULTBGMIGRATIONSPAUSED "0"
#define BGMIGRATIONSPAUSEDTAG "BG Migrations Paused"
#define BGMIGRATIONSPAUSEDHELP "If background migrations can run or  arepaused"
#define BGMIGRATIONSPAUSEDVALIDATESTRING 0
#define BGMIGRATIONSPAUSEDVALIDATEMIN 0 
#define BGMIGRATIONSPAUSEDVALIDATEMAX 1 
#define BGMIGRATIONSPAUSEDRESTARTREQUIRED 0 


// Define BGMIGRATIONMAXTHREADS
int BGMIGRATIONMAXTHREADS;  // the max number of JobScheduler threads
#define DEFAULTBGMIGRATIONMAXTHREADS "4"
#define BGMIGRATIONMAXTHREADSTAG "BG Migration Threads"
#define BGMIGRATIONMAXTHREADSHELP "The Maximum number of Background Migration Threads that can run concurrently"
#define BGMIGRATIONMAXTHREADSVALIDATESTRING 0
#define BGMIGRATIONMAXTHREADSVALIDATEMIN 0 
#define BGMIGRATIONMAXTHREADSVALIDATEMAX 32 
#define BGMIGRATIONMAXTHREADSRESTARTREQUIRED 1 

// Define BGMIGRATIONSTARTWINDOW
int BGMIGRATIONSTARTWINDOW;  // the max number of JobScheduler threads
#define DEFAULTBGMIGRATIONSTARTWINDOW "0000"
#define BGMIGRATIONSTARTWINDOWTAG "BG Migrations Start"
#define BGMIGRATIONSTARTWINDOWHELP "The earilest time of the day to run back ground migrations"
#define BGMIGRATIONSTARTWINDOWVALIDATESTRING 0
#define BGMIGRATIONSTARTWINDOWVALIDATEMIN 0 
#define BGMIGRATIONSTARTWINDOWVALIDATEMAX 2359 
#define BGMIGRATIONSTARTWINDOWRESTARTREQUIRED 0 

// Define BGMIRGATIONENDWINDOW
int BGMIGRATIONENDWINDOW;  // the max number of JobScheduler threads
#define DEFAULTBGMIGRATIONENDWINDOW "2359"
#define BGMIGRATIONENDWINDOWTAG "BG Migrations End"
#define BGMIGRATIONENDWINDOWHELP "The latested time of the day to run background migrations"
#define BGMIGRATIONENDWINDOWVALIDATESTRING 0
#define BGMIGRATIONENDWINDOWVALIDATEMIN 0 
#define BGMIGRATIONENDWINDOWVALIDATEMAX 2359 
#define BGMIGRATIONENDWINDOWRESTARTREQUIRED 0 

// Define BGMIGRATIONSLEEPMS
int BGMIGRATIONSLEEPMS;  // time in ms to sleep between each background migration 
#define DEFAULTBGMIGRATIONSLEEPMS "0"
#define BGMIGRATIONSLEEPMSTAG "BG Migration Sleep"
#define BGMIGRATIONSLEEPMSHELP "ms a wroker thread will sleep between background migration jobs"
#define BGMIGRATIONSLEEPMSVALIDATESTRING 0
#define BGMIGRATIONSLEEPMSVALIDATEMIN 0 
#define BGMIGRATIONSLEEPMSVALIDATEMAX 999999 
#define BGMIGRATIONSLEEPMSRESTARTREQUIRED 0 

// Define BGMIRGATIONT1FLOOR
int BGMIGRATIONT1FLOOR;  // the max number of JobScheduler threads
#define DEFAULTBGMIGRATIONT1FLOOR "0"
#define BGMIGRATIONT1FLOORTAG "BG Migrations Tier1 Floor Percentage"
#define BGMIGRATIONT1FLOORHELP "Percentange used that bg migrations should aim to reduce Tier-1 to during scheduled window (inconjuction with defined last used/accessed)"
#define BGMIGRATIONT1FLOORVALIDATESTRING 0
#define BGMIGRATIONT1FLOORVALIDATEMIN 0 
#define BGMIGRATIONT1FLOORVALIDATEMAX 100 
#define BGMIGRATIONT1FLOORRESTARTREQUIRED 0 

// Define BGMIRGATIONT1FLOOR
int BGMIGRATIONT2FLOOR;  // the max number of JobScheduler threads
#define DEFAULTBGMIGRATIONT2FLOOR "50"
#define BGMIGRATIONT2FLOORTAG "BG Migrations Tier2 Floor Percentage"
#define BGMIGRATIONT2FLOORHELP "Percentange used that bg migrations should aim to reduce Tier-2 to during scheduled window (inconjuction with defined last used/accessed)"
#define BGMIGRATIONT2FLOORVALIDATESTRING 0
#define BGMIGRATIONT2FLOORVALIDATEMIN 0 
#define BGMIGRATIONT2FLOORVALIDATEMAX 100 
#define BGMIGRATIONT2FLOORRESTARTREQUIRED 0 

// Define JSMAXTHREADS
int JSMAXTHREADS;  // the max number of JobScheduler threads
#define DEFAULTJSMAXTHREADS "4"
#define JSMAXTHREADSTAG "Schedular Threads"
#define JSMAXTHREADSHELP "The Maximum number of Job Scheduler Threads that can run concurrently"
#define JSMAXTHREADSVALIDATESTRING 0
#define JSMAXTHREADSVALIDATEMIN 0 
#define JSMAXTHREADSVALIDATEMAX 16 
#define JSMAXTHREADSRESTARTREQUIRED 1 

// Define JOBSCHEDULARSLEEP
int JOBSCHEDULARSLEEP;  // first highest pref local data storage location
#define DEFAULTJOBSCHEDULARSLEEP "60"
#define JOBSCHEDULARSLEEPTAG "Schedular Sleep"
#define JOBSCHEDULARSLEEPHELP "Seconds the internal jobs and bg migration scheduler should sleep between determining if a job should be started"
#define JOBSCHEDULARSLEEPVALIDATESTRING 0
#define JOBSCHEDULARSLEEPVALIDATEMIN 0 
#define JOBSCHEDULARSLEEPVALIDATEMAX 3600 
#define JOBSCHEDULARSLEEPRESTARTREQUIRED 0 

// Define JOBSCHEDULARSALLOWMANUALWHENPAUSED
int JOBSCHEDULARSALLOWMANUALWHENPAUSED;  // first highest pref local data storage location
#define DEFAULTJOBSCHEDULARALLOWMANUALWHENPAUSED "0"
#define JOBSCHEDULARALLOWMANUALWHENPAUSEDTAG "Schedular allow Manual"
#define JOBSCHEDULARALLOWMANUALWHENPAUSEDHELP "Allow Manual jobs to be run when the scheduler is paused"
#define JOBSCHEDULARALLOWMANUALWHENPAUSEDVALIDATESTRING 0
#define JOBSCHEDULARALLOWMANUALWHENPAUSEDVALIDATEMIN 0 
#define JOBSCHEDULARALLOWMANUALWHENPAUSEDVALIDATEMAX 1 
#define JOBSCHEDULARALLOWMANUALWHENPAUSEDRESTARTREQUIRED 0

// Define T1DDIR
char T1DDIR[2048];  // highest pref local data storage location
#define DEFAULTT1DDIR "/nexfs/t1"
#define T1DDIRTAG "Tier-1 data storage location"
#define T1DDIRHELP "Location of the highest preformance local storage to be used for data storage, ensure that no data remains in previuos location before restarting nexfs"
#define T1DDIRVALIDATESTRING 1
#define T1DDIRVALIDATEMIN 2 
#define T1DDIRVALIDATEMAX 2048 
#define T1DDIRRESTARTREQUIRED 1 

// Define T2DDIR
char T2DDIR[2048];  // second highest pref local data storage location
#define DEFAULTT2DDIR "/nexfs/t2"
#define T2DDIRTAG "Tier-2 data storage location"
#define T2DDIRHELP "Location of the second highest preformance local storage to be used for data storage, ensure that no data remains in previuos location before restarting nexfs"
#define T2DDIRVALIDATESTRING 1
#define T2DDIRVALIDATEMIN 2 
#define T2DDIRVALIDATEMAX 2048 
#define T2DDIRRESTARTREQUIRED 1 

// Define T1SDIR
 char T1SDIR[2048];  // high pref local for structure file storage location
#define DEFAULTT1SDIR "/nexfs/s1"
#define T1SDIRTAG "nexfs high pref structure file storage location"
#define T1SDIRHELP "Location local storage top be used for nexfs structure file storage, ensure all data moved before before restarting nexfs"
#define T1SDIRVALIDATESTRING 1
#define T1SDIRVALIDATEMIN 2 
#define T1SDIRVALIDATEMAX 2048 
#define T1SDIRRESTARTREQUIRED 1 


// Define T1DDIRENABLED
int T1DDIRENABLED;  // first highest pref local data storage location
#define DEFAULTT1DDIRENABLED "1"
#define T1DDIRENABLEDTAG "Enable the Tier-1 data storage location"
#define T1DDIRENABLEDHELP "Enable Tier-1 local storage"
#define T1DDIRENABLEDVALIDATESTRING 0
#define T1DDIRENABLEDVALIDATEMIN 0 
#define T1DDIRENABLEDVALIDATEMAX 1 
#define T1DDIRENABLEDRESTARTREQUIRED 1 

// Define T2DDIRENABLED
int T2DDIRENABLED;  // second highest pref local data storage location
#define DEFAULTT2DDIRENABLED "0"
#define T2DDIRENABLEDTAG "Enable the Tier-2 data storage location"
#define T2DDIRENABLEDHELP "Enable Tier-2 local storage"
#define T2DDIRENABLEDVALIDATESTRING 0
#define T2DDIRENABLEDVALIDATEMIN 0 
#define T2DDIRENABLEDVALIDATEMAX 1 
#define T2DDIRENABLEDRESTARTREQUIRED 1 

// Define T3DDIRENABLED
int T3DDIRENABLED;  // Tier-3 storage location (S3 Compabitable)
#define DEFAULTT3DDIRENABLED "1"
#define T3DDIRENABLEDTAG "Enable the Tier-3 S3 Compatible data storage location"
#define T3DDIRENABLEDHELP "Enable the Tier-3 S3 Compatible data storage location"
#define T3DDIRENABLEDVALIDATESTRING 0
#define T3DDIRENABLEDVALIDATEMIN 0 
#define T3DDIRENABLEDVALIDATEMAX 1 
#define T3DDIRENABLEDRESTARTREQUIRED 1 


// Define T1LOWWATERMARK
int T1LOWWATERMARK;  // Tier-1 dont use for reads that cannot already be served by T1 when this percent used T1 is hit, start migrating data to next tier  
#define DEFAULTT1LOWWATERMARK "70"
#define T1LOWWATERMARKTAG "Tier-1 Low Water Mark"
#define T1LOWWATERMARKHELP "Tier-1 Low Water Mark, serv only writes, and reads that can already be served by T1, new reads served by next available tier, start migrating data to next tier"
#define T1LOWWATERMARKVALIDATESTRING 0
#define T1LOWWATERMARKVALIDATEMIN 0 
#define T1LOWWATERMARKVALIDATEMAX 95
#define T1LOWWATERMARKRESTARTREQUIRED 0 

// Define T1HIGHWATERMARK
int T1HIGHWATERMARK;  // Tier-1 only serv reads and write for data already at T1 if percent used T1 is hit, start migrating data to next tier  
#define DEFAULTT1HIGHWATERMARK "80"
#define T1HIGHWATERMARKTAG "Tier-1 High Water Mark"
#define T1HIGHWATERMARKHELP "Tier-1 High Water Mark, serv only writes reads that can already be served by T1, start migrating data to next tier"
#define T1HIGHWATERMARKVALIDATESTRING 0
#define T1HIGHWATERMARKVALIDATEMIN 0 
#define T1HIGHWATERMARKVALIDATEMAX 90 
#define T1HIGHWATERMARKRESTARTREQUIRED 0 

// Define T2LOWWATERMARK
int T2LOWWATERMARK;  // Tier-2 dont use for reads that cannot already be served by T2 when this percent used T2 is hit, start migrating data to next tier  
#define DEFAULTT2LOWWATERMARK "80"
#define T2LOWWATERMARKTAG "Tier-2 Low Water Mark"
#define T2LOWWATERMARKHELP "Tier-2 Low Water Mark, serv only writes, and reads that can already be served by T2, other reads driect from next tier, start migrating data to next tier"
#define T2LOWWATERMARKVALIDATESTRING 0
#define T2LOWWATERMARKVALIDATEMIN 0 
#define T2LOWWATERMARKVALIDATEMAX 95
#define T2LOWWATERMARKRESTARTREQUIRED 0 

// Define T2HIGHWATERWATERMARK
int T2HIGHWATERMARK;  // Tier-1 only serv reads and write for data already at T1 if percent used T1 is hit, start migrating data to next tier  
#define DEFAULTT2HIGHWATERMARK "80"
#define T2HIGHWATERMARKTAG "Tier-2 High Water Mark"
#define T2HIGHWATERMARKHELP "Tier-2 High Water Mark, serv only writes reads that can already be served by T2, start migrating data to next tier"
#define T2HIGHWATERMARKVALIDATESTRING 0
#define T2HIGHWATERMARKVALIDATEMIN 0 
#define T2HIGHWATERMARKVALIDATEMAX 90 
#define T2HIGHWATERMARKRESTARTREQUIRED 0 

// Define SMARTPROTECTENABLED
int SMARTPROTECTENABLED;  // Enable Smart Protection
#define DEFAULTSMARTPROTECTENABLED "1"
#define SMARTPROTECTENABLEDTAG "Smart Protect Enabled"
#define SMARTPROTECTENABLEDHELP "Smart Protect Enabled, controlls both smartprotect and smart protect open file cycle, 0=off, 1=on"
#define SMARTPROTECTENABLEDVALIDATESTRING 0
#define SMARTPROTECTENABLEDVALIDATEMIN 0 
#define SMARTPROTECTENABLEDVALIDATEMAX 1 
#define SMARTPROTECTENABLEDRESTARTREQUIRED 0 

// Define SMARTPROTECT
int SMARTPROTECT;  // Continuous Protection
#define DEFAULTSMARTPROTECT "1"
#define SMARTPROTECTTAG "Smart Protect"
#define SMARTPROTECTHELP "Smart Protect Setting, 0=off, 1=on file close, 2=on data partclose, 3 on file and data part close"
#define SMARTPROTECTVALIDATESTRING 0
#define SMARTPROTECTVALIDATEMIN 0 
#define SMARTPROTECTVALIDATEMAX 3 
#define SMARTPROTECTRESTARTREQUIRED 0 

// Define SMARTPROTECTOPENFILECYCLE
int SMARTPROTECTOPENFILECYCLE;  // when to smart protect an open file
#define DEFAULTSMARTPROTECTOPENFILECYCLE "5"
#define SMARTPROTECTOPENFILECYCLETAG "Smart Protect Open File"
#define SMARTPROTECTOPENFILECYCLEHELP "How often in minutes to smart protect change datapart on a long time open file, 0=never, max=43200"
#define SMARTPROTECTOPENFILECYCLEVALIDATESTRING 0
#define SMARTPROTECTOPENFILECYCLEVALIDATEMIN 0 
#define SMARTPROTECTOPENFILECYCLEVALIDATEMAX 43200 
#define SMARTPROTECTOPENFILECYCLERESTARTREQUIRED 0 

// Define SMARTTIERENABLED
int SMARTTIERENABLED;  // Enable Tiering 
#define DEFAULTSMARTTIERENABLED "0"
#define SMARTTIERENABLEDTAG "Smart Tiering Enabled"
#define SMARTTIERENABLEDHELP "Smart Tiering Enabled, controlls both smart tier and smart tier open file cycle, 0=off, 1=on"
#define SMARTTIERENABLEDVALIDATESTRING 0
#define SMARTTIERENABLEDVALIDATEMIN 0 
#define SMARTTIERENABLEDVALIDATEMAX 1 
#define SMARTTIERENABLEDRESTARTREQUIRED 0 

// Define SMARTTIER
int SMARTTIER;  // Continuous Tiering
#define DEFAULTSMARTTIER "1"
#define SMARTTIERTAG "Smart Tier Mode"
#define SMARTTIERHELP "Smart Protect Setting, 0=off, 1=on file close, 2=on data partclose, 3 on file and data part close"
#define SMARTTIERVALIDATESTRING 0
#define SMARTTIERVALIDATEMIN 0 
#define SMARTTIERVALIDATEMAX 3 
#define SMARTTIERRESTARTREQUIRED 0 

// Define SMARTTIEROPENFILECYCLE
int SMARTTIEROPENFILECYCLE;  // when to scan for smart tier an open file
#define DEFAULTSMARTTIEROPENFILECYCLE "60"
#define SMARTTIEROPENFILECYCLETAG "Smart Tier Start Cycle" 
#define SMARTTIEROPENFILECYCLEHELP "How often in minutes to smart tier scans a open files dataparts, 0=never, max=43200"
#define SMARTTIEROPENFILECYCLEVALIDATESTRING 0
#define SMARTTIEROPENFILECYCLEVALIDATEMIN 0 
#define SMARTTIEROPENFILECYCLEVALIDATEMAX 43200 
#define SMARTTIEROPENFILECYCLERESTARTREQUIRED 0 

// Define NEXFSTIER1LASTWRITTEN
int NEXFSTIER1LASTWRITTEN;  // when to smart tier an open file
#define DEFAULTNEXFSTIER1LASTWRITTEN "1440"
#define NEXFSTIER1LASTWRITTENTAG "Smart Tier1 Last Written"
#define NEXFSTIER1LASTWRITTENHELP "How long in seconds after a datapart has been written and not accessed before it can be moved from tier1 or 2 storage, min=20, max=43200"
#define NEXFSTIER1LASTWRITTENVALIDATESTRING 0
#define NEXFSTIER1LASTWRITTENVALIDATEMIN 20 
#define NEXFSTIER1LASTWRITTENVALIDATEMAX 43200 
#define NEXFSTIER1LASTWRITTENRESTARTREQUIRED 0 

// Define NEXFSTIER1LASTACCESSED
int NEXFSTIER1LASTACCESSED;  // when to smart tier an open file
#define DEFAULTNEXFSTIER1LASTACCESSED "2880"
#define NEXFSTIER1LASTACCESSEDTAG "Smart Tier1 Last Accessed"
#define NEXFSTIER1LASTACCESSEDHELP "How long int seconds after a datapart was last accessed before it can be moved from tier1 or 2 storage, min=20, max=43200"
#define NEXFSTIER1LASTACCESSEDVALIDATESTRING 0
#define NEXFSTIER1LASTACCESSEDVALIDATEMIN 20 
#define NEXFSTIER1LASTACCESSEDVALIDATEMAX 43200 
#define NEXFSTIER1LASTACCESSEDRESTARTREQUIRED 0 

// Define NEXFSTIER2LASTWRITTEN
int NEXFSTIER2LASTWRITTEN;  // when to smart tier an open file
#define DEFAULTNEXFSTIER2LASTWRITTEN "7200"
#define NEXFSTIER2LASTWRITTENTAG "Smart Tier2 Last Written"
#define NEXFSTIER2LASTWRITTENHELP "How long int seconds after a datapart has been written and not accessed before it can be moved from tier2 storage, 0=never, max=43200"
#define NEXFSTIER2LASTWRITTENVALIDATESTRING 0
#define NEXFSTIER2LASTWRITTENVALIDATEMIN 0 
#define NEXFSTIER2LASTWRITTENVALIDATEMAX 43200 
#define NEXFSTIER2LASTWRITTENRESTARTREQUIRED 0 

// Define NEXFSTIER2LASTACCESSED
int NEXFSTIER2LASTACCESSED;  // when to smart tier an open file
#define DEFAULTNEXFSTIER2LASTACCESSED "20160"
#define NEXFSTIER2LASTACCESSEDTAG "Smart Tier2 Last Accessed"
#define NEXFSTIER2LASTACCESSEDHELP "How long in seconds after a datapart was last accessed before it can be moved from tier2 storage, 0=never, max=43200"
#define NEXFSTIER2LASTACCESSEDVALIDATESTRING 0
#define NEXFSTIER2LASTACCESSEDVALIDATEMIN 0 
#define NEXFSTIER2LASTACCESSEDVALIDATEMAX 43200 
#define NEXFSTIER2LASTACCESSEDRESTARTREQUIRED 0 

// Define RETIERORDER
int RETIERORDER;  // order to retier from tier3 on read
#define DEFAULTRETIERORDER "12"
#define RETIERORDERTAG "Re-Tier on read/write tier order"
#define RETIERORDERHELP "Which tier to move dataparts too when they become active again after move to tier-3"
#define RETIERORDERVALIDATESTRING 0
#define RETIERORDERVALIDATEMIN 0 
#define RETIERORDERVALIDATEMAX 21 
#define RETIERORDERRESTARTREQUIRED 0 

// Define RETIERORDER
int RETIERONREAD;  // retier datafile on read from slower tier
#define DEFAULTRETIERONREAD "0"
#define RETIERONREADTAG "Re-Tier On Read"
#define RETIERONREADHELP "If reads requests from a slower tier should tigger datafile migration to fast tiers"
#define RETIERONREADVALIDATESTRING 0
#define RETIERONREADVALIDATEMIN 0 
#define RETIERONREADVALIDATEMAX 1 
#define RETIERONREADRESTARTREQUIRED 0 

// Define PARTSIZE
int PARTSIZE;  // data file size
#define DEFAULTPARTSIZE "1024000"
#define PARTSIZETAG "Part Size"
#define PARTSIZEHELP "Data file Part size in bytes, files are split into chunks of this size for migration to slower tiers, NOTE: existing files will retain the partsize configured when first created"
#define PARTSIZEVALIDATESTRING 0
#define PARTSIZEVALIDATEMIN 512 
#define PARTSIZEVALIDATEMAX 8192000 
#define PARTSIZERESTARTREQUIRED 0 

// Define WRITEBUFSIZE
int WRITEBUFSIZE;  // Size of incoming write buffer (for receiving data from the kernel)
#define DEFAULTWRITEBUFSIZE "4096000"
#define WRITEBUFSIZETAG "Write Buffer Size"
#define WRITEBUFSIZEHELP "Write buffer size in bytes, Size of incoming write buffer (for receiving data from the kernel)"
#define WRITEBUFSIZEVALIDATESTRING 0
#define WRITEBUFSIZEVALIDATEMIN 512 
#define WRITEBUFSIZEVALIDATEMAX 4096000 
#define WRITEBUFSIZERESTARTREQUIRED 0 

// Define WRITEBUFASYNC /// DO NOT ENABLE, NOT WORKING
int WRITEBUFASYNC;  // write changed buffer content to disk on part change or close 
#define DEFAULTWRITEBUFASYNC "0"
#define WRITEBUFASYNCTAG "Write Buffer Async"
#define WRITEBUFASYNCHELP "Write buffer Async, write buffer changes to disk on part change (default) or as changes are received"
#define WRITEBUFASYNCVALIDATESTRING 0
#define WRITEBUFASYNCVALIDATEMIN 0 
#define WRITEBUFASYNCVALIDATEMAX 1 
#define WRITEBUFASYNCRESTARTREQUIRED 1 

// Define MAXFILENAMELENGTH
int MAXFILENAMELENGTH;  // Max support file name length (number of chars)
#define DEFAULTMAXFILENAMELENGTH "2000"
//#define DEFAULTMAXFILENAMELENGTH "8192"
#define MAXFILENAMELENGTHTAG "Max File Name Length"
#define MAXFILENAMELENGTHHELP "Max number of chars allowed in a file name including full path, currently limited to 2000 chars, will be increased to 65K in upcoming release"
#define MAXFILENAMELENGTHVALIDATESTRING 0
#define MAXFILENAMELENGTHVALIDATEMIN 1024 
#define MAXFILENAMELENGTHVALIDATEMAX 2000 
// #define MAXFILENAMELENGTHVALIDATEMAX 65536 
#define MAXFILENAMELENGTHRESTARTREQUIRED 1 

// Define T3S3ENABLEMD5DIGEST 
int T3S3ENABLEMD5DIGEST;  // S3 Enable MD5 Digest on PUT & POST
#define DEFAULTT3S3ENABLEMD5DIGEST "1"
#define T3S3ENABLEMD5DIGESTTAG "T3 S3 include MD5 Digest"
#define T3S3ENABLEMD5DIGESTHELP "Include data MD5 Digest on T3 S3 PUTS & POSTS"
#define T3S3ENABLEMD5DIGESTVALIDATESTRING 0
#define T3S3ENABLEMD5DIGESTVALIDATEMIN 0
#define T3S3ENABLEMD5DIGESTVALIDATEMAX 1
#define T3S3ENABLEMD5DIGESTRESTARTREQUIRED 0

// Define T3USEHTTPS 
int T3USEHTTPS;  // Connect to T3 storage using HTTPS 
#define DEFAULTT3USEHTTPS "1"
#define T3USEHTTPSTAG "T3 use HTTPS"
#define T3USEHTTPSHELP "Use HTTPS to connect to the T3 storage endpoint"
#define T3USEHTTPSVALIDATESTRING 0
#define T3USEHTTPSVALIDATEMIN 0
#define T3USEHTTPSVALIDATEMAX 1
#define T3USEHTTPSRESTARTREQUIRED 0

// Define T3S3COMPATIBLE 
int T3S3COMPATIBLE;  // Connect to T3 storage using HTTPS 
#define DEFAULTT3S3COMPATIBLE "0"
#define T3S3COMPATIBLETAG "T3 S3 compatible"
#define T3S3COMPATIBLEHELP "T3 storage is S3 Compatible"
#define T3S3COMPATIBLEVALIDATESTRING 0
#define T3S3COMPATIBLEVALIDATEMIN 0
#define T3S3COMPATIBLEVALIDATEMAX 1
#define T3S3COMPATIBLERESTARTREQUIRED 0

// Define T3S3URL
char T3S3URL[2048];  // S3 compatible storage URL 
#define DEFAULTT3S3URL "amazonaws.com"
#define T3S3URLTAG "T3 S3 URL"
#define T3S3URLHELP "URL for accessing T3 S3 compatible storage" 
#define T3S3URLVALIDATESTRING 1
#define T3S3URLVALIDATEMIN 0
#define T3S3URLVALIDATEMAX 2048
#define T3S3URLRESTARTREQUIRED 0

// Define T3S3USEVIRTUALHOST 
int T3S3USEVIRTUALHOST;  // Connect to S3 storage using AWS Virtual Host style (or path style) 
#define DEFAULTT3S3USEVIRTUALHOST "1"
#define T3S3USEVIRTUALHOSTTAG "Use T3 S3 Virtual Host Style"
#define T3S3USEVIRTUALHOSTHELP "Use T3 S3 Virtual Host Style (otherwsie path style) to connect to the S3 storage endpoint"
#define T3S3USEVIRTUALHOSTVALIDATESTRING 0
#define T3S3USEVIRTUALHOSTVALIDATEMIN 0
#define T3S3USEVIRTUALHOSTVALIDATEMAX 1
#define T3S3USEVIRTUALHOSTRESTARTREQUIRED 0

// Define T3S3SIGNATUREV4 
int T3S3SIGNATUREV4;  // Use AWS Singnature V4 (or V2) 
#define DEFAULTT3S3SIGNATUREV4 "1"
#define T3S3SIGNATUREV4TAG "Use T3 AWS Signature Version 4"
#define T3S3SIGNATUREV4HELP "Use T3 AWS Signature Version 4 (otherwise V2) to connect to the S3 storage endpoint"
#define T3S3SIGNATUREV4VALIDATESTRING 0
#define T3S3SIGNATUREV4VALIDATEMIN 0
#define T3S3SIGNATUREV4VALIDATEMAX 1
#define T3S3SIGNATUREV4RESTARTREQUIRED 0

// Define T3S3ENABLEREGION 
int T3S3ENABLEREGION;  // Include S3 Region with connecting to S3 storage 
#define DEFAULTT3S3ENABLEREGION "1"
#define T3S3ENABLEREGIONTAG "T3 S3 enable Region"
#define T3S3ENABLEREGIONHELP "Use the configured T3 S3 REGION to connect to the S3 storage endpoint"
#define T3S3ENABLEREGIONVALIDATESTRING 0
#define T3S3ENABLEREGIONVALIDATEMIN 0
#define T3S3ENABLEREGIONVALIDATEMAX 1
#define T3S3ENABLEREGIONRESTARTREQUIRED 0

// Define T3S3REGION
char T3S3REGION[2048];  // The S3 REGION to use 
#define DEFAULTT3S3REGION "us-west-1"
#define T3S3REGIONTAG "T3 S3 Region"
#define T3S3REGIONHELP "The AWS T3 S3 Region"
#define T3S3REGIONVALIDATESTRING 1
#define T3S3REGIONVALIDATEMIN 5
#define T3S3REGIONVALIDATEMAX 2048
#define T3S3REGIONRESTARTREQUIRED 0

// Define T3S3BUCKET
char T3S3BUCKET[2048];  // The S3 Bucket to use 
#define DEFAULTT3S3BUCKET "nexfs"
#define T3S3BUCKETTAG "T3 S3 Bucket"
#define T3S3BUCKETHELP "The T3 S3 Bucket to use"
#define T3S3BUCKETVALIDATESTRING 1
#define T3S3BUCKETVALIDATEMIN 0
#define T3S3BUCKETVALIDATEMAX 2048
#define T3S3BUCKETRESTARTREQUIRED 0

// Define T3S3PORT
int T3S3PORT;  // The S3 Port to use
#define DEFAULTT3S3PORT "9000"
#define T3S3PORTTAG "T3 S3 Port"
#define T3S3PORTHELP "The TCP T3 S3 Port"
#define T3S3PORTVALIDATESTRING 0
#define T3S3PORTVALIDATEMIN 0
#define T3S3PORTVALIDATEMAX 16384
#define T3S3PORTRESTARTREQUIRED 0

// Define T3S3RETRY404
int T3S3RETRY404;  // Time to wait before retring on receiving a 404 to a S3 request
#define DEFAULTT3S3RETRY404 "0"
#define T3S3RETRY404TAG "T3 S3 Retry 404"
#define T3S3RETRY404HELP "Retry the T3 S3 request on receiving a 404 (may impact read/write preforamnce)"
#define T3S3RETRY404VALIDATESTRING 0
#define T3S3RETRY404VALIDATEMIN 0
#define T3S3RETRY404VALIDATEMAX 1
#define T3S3RETRY404RESTARTREQUIRED 0

// Define T3S3RETRY404SLEEP
int  T3S3RETRY404SLEEP;  // Time to wait before retring a 404 T3 S3 request
#define DEFAULTT3S3RETRY404SLEEP "0"
#define T3S3RETRY404SLEEPTAG "T3 S3 404 Retry ms Sleep"
#define T3S3RETRY404SLEEPHELP "The time in ms to wait before retring 404 T3 S3 request, a value > 0 will impact performance"
#define T3S3RETRY404SLEEPVALIDATESTRING 0
#define T3S3RETRY404SLEEPVALIDATEMIN 0
#define T3S3RETRY404SLEEPVALIDATEMAX 999999
#define T3S3RETRY404SLEEPRESTARTREQUIRED 0

// Define T3S3RETRYSLEEP
int T3S3RETRYSLEEP;  // Time to wait before retring a failed S3 request
#define DEFAULTT3S3RETRYSLEEP "1"
#define T3S3RETRYSLEEPTAG "T3 S3 Retry Sleep"
#define T3S3RETRYSLEEPHELP "The time in seconds to wait before retring a failed S3 request"
#define T3S3RETRYSLEEPVALIDATESTRING 0
#define T3S3RETRYSLEEPVALIDATEMIN 0
#define T3S3RETRYSLEEPVALIDATEMAX 360
#define T3S3RETRYSLEEPRESTARTREQUIRED 0

// Define T3S3RETRIES
int T3S3RETRIES;  // Time to wait before retring a failed S3 request
#define DEFAULTT3S3RETRIES "3"
#define T3S3RETRIESTAG "T3 S3 Retries"
#define T3S3RETRIESHELP "The maximum number of times to retry a failing T3 S3 request"
#define T3S3RETRIESVALIDATESTRING 0
#define T3S3RETRIESVALIDATEMIN 0
#define T3S3RETRIESVALIDATEMAX 12
#define T3S3RETRIESRESTARTREQUIRED 0

// Define T3AWSSecretAccessKey
char T3AWSSecretAccessKey[2048];  // The Secret Access Key 
#define DEFAULTT3AWSSecretAccessKey "minioadmin"
#define T3AWSSecretAccessKeyTAG "AWSSecretAccessKey"
#define T3AWSSecretAccessKeyHELP "The Secret Access Key to using for accessing the S3 compatible storage"
#define T3AWSSecretAccessKeyVALIDATESTRING 1
#define T3AWSSecretAccessKeyVALIDATEMIN 0
#define T3AWSSecretAccessKeyVALIDATEMAX 2048
#define T3AWSSecretAccessKeyRESTARTREQUIRED 0

// Define T3AWSAccessKeyId
char T3AWSAccessKeyId[2048];  // The AWS Access Key Id 
#define DEFAULTT3AWSAccessKeyId "minioadmin"
#define T3AWSAccessKeyIdTAG "AWSSecretAccessKey"
#define T3AWSAccessKeyIdHELP "The Secret Access Key to using for accessing the S3 compatible storage"
#define T3AWSAccessKeyIdVALIDATESTRING 1
#define T3AWSAccessKeyIdVALIDATEMIN 0
#define T3AWSAccessKeyIdVALIDATEMAX 2048
#define T3AWSAccessKeyIdRESTARTREQUIRED 0

// Define MOUNTPOINT
char MOUNTPOINT[2048];  // The nexfs mountpoint, needed for nexfscli to communicate with the nexfs storage server
#define DEFAULTMOUNTPOINT "/mnt/nexfs"
#define MOUNTPOINTTAG "MOUNTPOINT"
#define MOUNTPOINTHELP "NEXFS Mountpoint, needed for nexfscli to work, must match the nexfs mountpoint although this setting is not used here by the nexfs storgae server itself"
#define MOUNTPOINTVALIDATESTRING 1
#define MOUNTPOINTVALIDATEMIN 2
#define MOUNTPOINTVALIDATEMAX 2048
#define MOUNTPOINTRESTARTREQUIRED 0

// Define GFSLOGLEVEL
char GFSLOGLEVEL[8];
#define DEFAULTGFSLOGLEVEL "NOTICE"
#define GFSLOGLEVELTAG "LOGLEVEL"
#define GFSLOGLEVELHELP "The Loglevel, one of CRIT, ERR, WARNING, NOTICE, INFO or DEBUG"
#define GFSLOGLEVELVALIDATESTRING 1
#define GFSLOGLEVELVALIDATEMIN 2
#define GFSLOGLEVELVALIDATEMAX 8
#define GFSLOGLEVELRESTARTREQUIRED 0

// Define SYSLOGFACILITY
char SYSLOGFACILITY[11];
#define DEFAULTSYSLOGFACILITY "LOG_USER"
#define SYSLOGFACILITYTAG "SYSLOGFACILITY"
#define SYSLOGFACILITYHELP "The Syslog Facility to use, one of LOG_USER (default), LOG_LOCAL0...7"
#define SYSLOGFACILITYVALIDATESTRING 1
#define SYSLOGFACILITYVALIDATEMIN 2
#define SYSLOGFACILITYVALIDATEMAX 10 
#define SYSLOGFACILITYRESTARTREQUIRED 0


// Define MAXHANDLEPAGES
int MAXHANDLEPAGES;
#define DEFAULTMAXHANDLEPAGES "4"
#define MAXHANDLEPAGESTAG "MAXHANDLEPAGES"
#define MAXHANDLEPAGESHELP "The maximum number of cache pages that can exist for a open file handle, higher number increases concurrency and memory usage"
#define MAXHANDLEPAGESVALIDATESTRING 0
#define MAXHANDLEPAGESVALIDATEMIN 1
#define MAXHANDLEPAGESVALIDATEMAX 128
#define MAXHANDLEPAGESRESTARTREQUIRED 1

// Define CACHELOCKWAIT
int CACHELOCKWAIT;
#define DEFAULTCACHELOCKWAIT "4000"
#define CACHELOCKWAITTAG "CACHELOCKWAIT"
#define CACHELOCKWAITHELP "The number of nano seconds to wait between attempts to take a buffer lock"
#define CACHELOCKWAITVALIDATESTRING 0
#define CACHELOCKWAITVALIDATEMIN 5
#define CACHELOCKWAITVALIDATEMAX 99999999
#define CACHELOCKWAITRESTARTREQUIRED 0

// Define ROOTONLYACCESS
int ROOTONLYACCESS;
#define DEFAULTROOTONLYACCESS "0"
#define ROOTONLYACCESSTAG "ROOTONLYACCESS"
#define ROOTONLYACCESSHELP "If only the root user can access the nexfs filesystem, or if access is allowed for all users (When nexfs started using nexfscli)"
#define ROOTONLYACCESSVALIDATESTRING 0
#define ROOTONLYACCESSVALIDATEMIN 0
#define ROOTONLYACCESSVALIDATEMAX 1
#define ROOTONLYACCESSRESTARTREQUIRED 1

// Define DEFAULTPERMISSIONS
int DEFAULTPERMISSIONS; // if to set mount option default_permissions;
#define DEFAULTDEFAULTPERMISSIONS "1"
#define DEFAULTPERMISSIONSTAG "DEFAULTPERMISSIONS"
#define DEFAULTPERMISSIONSHELP "Enables permission checking, restricting access based on file mode"
#define DEFAULTPERMISSIONSVALIDATESTRING 0
#define DEFAULTPERMISSIONSVALIDATEMIN 0
#define DEFAULTPERMISSIONSVALIDATEMAX 1
#define DEFAULTPERMISSIONSRESTARTREQUIRED 1

// Define NEXFSCMD
char NEXFSCMD[2048];
#define DEFAULTNEXFSCMD "nexfs.server"
#define NEXFSCMDTAG "nexfs.server path including binary"
#define NEXFSCMDHELP "nexfs.server binary including path if not in default search path, used by nexfscli"
#define NEXFSCMDVALIDATESTRING 1
#define NEXFSCMDVALIDATEMIN 5
#define NEXFSCMDVALIDATEMAX 2048
#define NEXFSCMDRESTARTREQUIRED 0

// Define CACHELOCKWAIT
int DELETEMOVETOTIER3;
#define DEFAULTDELETEMOVETOTIER3 "1"
#define DELETEMOVETOTIER3TAG "DELETEMOVETOTIER3" 
#define DELETEMOVETOTIER3HELP "Moved Deleted datafiles to tier3 while waiting for final deletion"
#define DELETEMOVETOTIER3VALIDATESTRING 0
#define DELETEMOVETOTIER3VALIDATEMIN 0
#define DELETEMOVETOTIER3VALIDATEMAX 1
#define DELETEMOVETOTIER3RESTARTREQUIRED 0

// Define DELETERETAINDAYS
int DELETERETAINDAYS;
#define DEFAULTDELETERETAINDAYS "7"
#define DELETERETAINDAYSTAG "DELETERETAINDAYS" 
#define DELETERETAINDAYSHELP "Days to retain deleted datafiles before final deletion"
#define DELETERETAINDAYSVALIDATESTRING 0
#define DELETERETAINDAYSVALIDATEMIN 0
#define DELETERETAINDAYSVALIDATEMAX 10000
#define DELETERETAINDAYSRESTARTREQUIRED 0

// Define DELETEPAUSED 
int DELETEPAUSED;  // if background deletions should run or be paused
#define DEFAULTDELETEPAUSED "0"
#define DELETEPAUSEDTAG "Deletions Paused"
#define DELETEPAUSEDHELP "If background deletions can run or are paused"
#define DELETEPAUSEDVALIDATESTRING 0
#define DELETEPAUSEDVALIDATEMIN 0 
#define DELETEPAUSEDVALIDATEMAX 1 
#define DELETEPAUSEDRESTARTREQUIRED 0 

// Define DELETEMAXTHREADS
int DELETEMAXTHREADS;  // the max number of JobScheduler threads
#define DEFAULTDELETEMAXTHREADS "4"
#define DELETEMAXTHREADSTAG "BG Deletion Threads"
#define DELETEMAXTHREADSHELP "The Maximum number of Background Deletions Threads that can run concurrently"
#define DELETEMAXTHREADSVALIDATESTRING 0
#define DELETEMAXTHREADSVALIDATEMIN 0 
#define DELETEMAXTHREADSVALIDATEMAX 32 
#define DELETEMAXTHREADSRESTARTREQUIRED 1 

// Define DELETESTARTWINDOW
int DELETESTARTWINDOW;  // Time of day to start bg deletions
#define DEFAULTDELETESTARTWINDOW "0000"
#define DELETESTARTWINDOWTAG "BG Deletions Start"
#define DELETESTARTWINDOWHELP "The earliest time of the day to run back ground deletions"
#define DELETESTARTWINDOWVALIDATESTRING 0
#define DELETESTARTWINDOWVALIDATEMIN 0 
#define DELETESTARTWINDOWVALIDATEMAX 2359 
#define DELETESTARTWINDOWRESTARTREQUIRED 0 

// Define DELETEENDWINDOW
int DELETEENDWINDOW;  // Time of day to stop/pause bg deletions
#define DEFAULTDELETEENDWINDOW "2359"
#define DELETEENDWINDOWTAG "BG Deletions End"
#define DELETEENDWINDOWHELP "The latest time of the day to run background deletions"
#define DELETEENDWINDOWVALIDATESTRING 0
#define DELETEENDWINDOWVALIDATEMIN 0 
#define DELETEENDWINDOWVALIDATEMAX 2359 
#define DELETEENDWINDOWRESTARTREQUIRED 0 

// Define DELETSLEEPMS
int DELETESLEEPMS;  // time in ms to sleep between each background deletion 
#define DEFAULTDELETESLEEPMS "0"
#define DELETESLEEPMSTAG "BG Deletion Sleep"
#define DELETESLEEPMSHELP "ms a worker thread will sleep between background migration jobs"
#define DELETESLEEPMSVALIDATESTRING 0
#define DELETESLEEPMSVALIDATEMIN 0 
#define DELETESLEEPMSVALIDATEMAX 999999 
#define DELETESLEEPMSRESTARTREQUIRED 0 

// Define TRUNCTATE0ASDELETE
int TRUNCTATE0ASDELETE;
#define DEFAULTTRUNCTATE0ASDELETE "1"
#define TRUNCTATE0ASDELETETAG "Truncate 0 as Delete" 
#define TRUNCTATE0ASDELETEHELP "Treat full file truncates as datafile deletes, or purge datafile on truncate"
#define TRUNCTATE0ASDELETEVALIDATESTRING 0
#define TRUNCTATE0ASDELETEVALIDATEMIN 0
#define TRUNCTATE0ASDELETEVALIDATEMAX 1
#define TRUNCTATE0ASDELETERESTARTREQUIRED 0

// Define AUTORELESEDATAFILEMS 
int AUTORELESEDATAFILEMS;
#define DEFAULTAUTORELESEDATAFILEMS "80"
#define AUTORELESEDATAFILEMSTAG "Auto Release Datafile (ms)" 
#define AUTORELESEDATAFILEMSHELP "How long to keep an idle datafile open for closure (in ms)"
#define AUTORELESEDATAFILEMSVALIDATESTRING 0
#define AUTORELESEDATAFILEMSVALIDATEMIN 50
#define AUTORELESEDATAFILEMSVALIDATEMAX 5000
#define AUTORELESEDATAFILEMSRESTARTREQUIRED 0

// Define SECURITYCMDGROUP 
int SECURITYCMDGROUP;
#define DEFAULTSECURITYCMDGROUP "0"
#define SECURITYCMDGROUPTAG "Securitry CMD Group" 
#define SECURITYCMDGROUPHELP "User Group whos members can run nexfs admin commands"
#define SECURITYCMDGROUPVALIDATESTRING 0
#define SECURITYCMDGROUPVALIDATEMIN 0
#define SECURITYCMDGROUPVALIDATEMAX 65536
#define SECURITYCMDGROUPRESTARTREQUIRED 0

// Define SECURITYCMDGROUPACCESS
int SECURITYCMDGROUPACCESS;
#define DEFAULTSECURITYCMDGROUPACCESS "2"
#define SECURITYCMDGROUPACCESSTAG "Securitry CMD Group" 
#define SECURITYCMDGROUPACCESSHELP "User Group whos members can run nexfs admin commands, 0=no access, 1 read only, 2 read/write"
#define SECURITYCMDGROUPACCESSVALIDATESTRING 0
#define SECURITYCMDGROUPACCESSVALIDATEMIN 0
#define SECURITYCMDGROUPACCESSVALIDATEMAX 2
#define SECURITYCMDGROUPACCESSRESTARTREQUIRED 0

// Define SECURITYCONFGROUP 
int SECURITYCONFGROUP;
#define DEFAULTSECURITYCONFGROUP "0"
#define SECURITYCONFGROUPTAG "Securitry CMD Group" 
#define SECURITYCONFGROUPHELP "User Group whos members can run nexfs admin commands"
#define SECURITYCONFGROUPVALIDATESTRING 0
#define SECURITYCONFGROUPVALIDATEMIN 0
#define SECURITYCONFGROUPVALIDATEMAX 65536
#define SECURITYCONFGROUPRESTARTREQUIRED 0

// Define SECURITYCONFGROUPACCESS
int SECURITYCONFGROUPACCESS;
#define DEFAULTSECURITYCONFGROUPACCESS "2"
#define SECURITYCONFGROUPACCESSTAG "Securitry Configuration Group" 
#define SECURITYCONFGROUPACCESSHELP "User Group whos members can view and make nexfs configuration changes, 0=no access, 1 read only, 2 read/write"
#define SECURITYCONFGROUPACCESSVALIDATESTRING 0
#define SECURITYCONFGROUPACCESSVALIDATEMIN 0
#define SECURITYCONFGROUPACCESSVALIDATEMAX 2
#define SECURITYCONFGROUPACCESSRESTARTREQUIRED 0

// Define NEXFSLICENSEKEY
char NEXFSLICENSEKEY[4096];
#define DEFAULTNEXFSLICENSEKEY "0"
#define NEXFSLICENSEKEYTAG "Nexfs License"
#define NEXFSLICENSEKEYHELP "The Installed Nexfs License" 
#define NEXFSLICENSEKEYVALIDATESTRING 1
#define NEXFSLICENSEKEYVALIDATEMIN 1
#define NEXFSLICENSEKEYVALIDATEMAX 4096 
#define NEXFSLICENSEKEYRESTARTREQUIRED 0 

// Define NEXFSAPICHANGECMDGROUP
int NEXFSAPICHANGECMDGROUP;
#define DEFAULTNEXFSAPICHANGECMDGROUP "0"
#define NEXFSAPICHANGECMDGROUPTAG "NEXFSAPICHANGECMDGROUP"
#define NEXFSAPICHANGECMDGROUPHELP "" 
#define NEXFSAPICHANGECMDGROUPVALIDATESTRING 0
#define NEXFSAPICHANGECMDGROUPVALIDATEMIN 0
#define NEXFSAPICHANGECMDGROUPVALIDATEMAX 65536 
#define NEXFSAPICHANGECMDGROUPRESTARTREQUIRED 0

// Define NEXFSAPICHANGECONFGROUP
int NEXFSAPICHANGECONFGROUP;
#define DEFAULTNEXFSAPICHANGECONFGROUP "0"
#define NEXFSAPICHANGECONFGROUPTAG "NEXFSAPICHANGECONFGROUP"
#define NEXFSAPICHANGECONFGROUPHELP "" 
#define NEXFSAPICHANGECONFGROUPVALIDATESTRING 0
#define NEXFSAPICHANGECONFGROUPVALIDATEMIN 0
#define NEXFSAPICHANGECONFGROUPVALIDATEMAX 65536 
#define NEXFSAPICHANGECONFGROUPRESTARTREQUIRED 0

