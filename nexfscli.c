// Copyright (c) 2021 Nexustorage Limited.
// nexfscli.c Nexustorage nexfs commandline interface
//
// This file is part of Nexustorage Nexfs Storage stack
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/xattr.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <grp.h>
#include <ctype.h>
#include "gfsconf.h"
#include "gfsconf_funcs.h"
#include "gfslogging.h"

#define NEXFSCLIRELEASE "0.4dev1"
#define QUEUELIST 1 


void print_connecterr()
{
  printf("Connection to newfs server failed, check any passed options are correct or server status by running: nexfscli server status\n");
}

int generateconfigfiles(char *argv[])
{
  int res=0;
  char basefoldername[260020];

  printf("Generating Configuration Files\n");

  res=gfs_loadconfig(1);
 
  if ( res == 0 )
  {
    snprintf(basefoldername,260016,"%s/%s",DEFAULTGFSCONFDIR,GFSCONFIGTAG);
    res = mkdir(basefoldername, 0700);
    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create config directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
      return -errno;
    }

    if ( errno != EEXIST )
    {
      res=chown(basefoldername, 0, 0);
      if ( res != 0 )
      {
        printf("%s: Failed to create config directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
        return -errno;
      }
    }

    errno=0;
    snprintf(basefoldername,260016,"%s/%s",DEFAULTGFSCONFDIR,GFSCMDTAG);
    res = mkdir(basefoldername, 0700);

    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create config directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
      return -errno;
    }

    if (( errno == 0 ) || ( errno != EEXIST ))
    {
      res=chown(basefoldername, 0, 0);
      if (( res != 0 )  || ( errno != EEXIST ))
      {
        printf("%s: Failed to create config directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
        return -errno;
      }
    }

    printf("Done.\n");
  }
  else
  {
    printf(".. Returned %d - FAILED, hints: does /etc/nexfs exist, have you run nexfscli -init?\n",res);
    return -1;
  }

  return 0;
} 

int printconfigtags()
{
  int loop=0;

  printf("Valid ConfigTag\n");

  for (loop=0; loop<GFSCONFIGTAGSCOUNT; loop++)
    printf("%s\n",GFSCONFIGTAGS[loop]);

  return 0;
}

int getliveconfig(char *CONFTAG, char *valuebuf, int bufsize)
{
  int res=0;
  int lcp;
  char returnbuf[4096];
  char TAGPATH[4500];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",returnbuf,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,4500,"%s/%s/%s",returnbuf,GFSCONFIGTAG,CONFTAG);

  lcp = open( TAGPATH, O_RDONLY); 

  if (lcp == -1) 
  {
    print_connecterr();
    return -errno;
  }

  res = pread(lcp, valuebuf, bufsize, 0);
  valuebuf[res]=0;
  close(lcp);

  if (res == -1)
  {
     printf("ERR: failed to read value with errno %d - %s ",errno,strerror(errno));
     return -errno;
  } 

  return res;
}

int releaseinfo(int argc, char *argv[])
{
  int res=0;
  char returnbuf[16];

  if ( argc < 4 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[2],"get") == 0 )
  {
    if ( strcmp(argv[3],"nexfscli") == 0 )
    {
      printf("%s\n",NEXFSCLIRELEASE);
      return 0;
    }
    else if ( strcmp(argv[3],"nexfs") == 0 )  
    {
      if (( res=getliveconfig("NEXFSRELEASE", returnbuf, 16) ) < 0 ) return(res);

      printf("%s\n",returnbuf);
      return res;
    }
  }
  printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
  return -1;
}

int getserverstats()
{
  char buf[65536] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,2224,"%s/%s/getstats",mountpoint,GFSCMDTAG);

  lcp = open( TAGPATH, O_RDWR|O_DIRECT ); 

  if (lcp == -1) 
  {
    print_connecterr();
    return -errno;
  }

  strncpy(buf,"",65536);

  res = pwrite(lcp, buf,65536 ,0 );
  if (res < 0 )
  {
     printf("ERR: failed to send cmd to nexfs with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = pread(lcp,buf,65536,0);

  if ( res < 0 )
  {
     printf("ERR: failed to read stats from nexfs errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }

  printf("%s",buf);

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request server stats errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;
}

int showfiletierusage(char *filename,int option )
{
  char buf[65536] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  if ( option == 0 )
  {
    snprintf(TAGPATH,2224,"%s/%s/getfiletierinfo",mountpoint,GFSCMDTAG);
  }
  else
  {
    snprintf(TAGPATH,2224,"%s/%s/getextendedfiletierinfo",mountpoint,GFSCMDTAG);
  }

  lcp = open( TAGPATH, O_RDWR|O_DIRECT ); 

  if (lcp == -1) 
  {
    print_connecterr();
    return -errno;
  }

  strncpy(buf,filename,65536);

  res = pwrite(lcp, buf,65536 ,0 );
  if (res < 0 )
  {
     printf("ERR: failed to send filename with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = pread(lcp,buf,65536,0);

  if ( res < 0 )
  {
     printf("ERR: failed to read file tier info with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }

  printf("%s",buf);

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request tier move with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;

}

int movetotier(int tier,char *filetomove)
{
  int res=0;
  int lcp;
  char returnbuf[2048];
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",returnbuf,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",res,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,2224,"%s/%s/movetotier%d",returnbuf,GFSCMDTAG,tier);

  lcp = open( TAGPATH, O_RDWR|O_DIRECT); 
  
  if (lcp == -1) 
  {
    print_connecterr();
    return -errno;
  }

  res = pwrite(lcp, filetomove, strlen(filetomove),0 );
  if (res < 0 )
  {
    if ( errno == EBUSY )
      printf("ERR: Failed to submit job, nexfs jobqueue is full \n");
    else
      printf("ERR: failed to send filename with errno %d - %s\n",errno,strerror(errno));

    return -errno;
  } 

  res = pread(lcp, returnbuf,2048,0 );
  if (res < 0 )
  {
     printf("ERR: failed to retrive a jobid with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request tier move with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  printf("Submitted JobID: %s\n",returnbuf);
  return 0;
}

int setliveconfig(char *CONFTAG, char *valuebuf, int bufsize)
{
  int res=0;
  int lcp;
  char returnbuf[2048];
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",returnbuf,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,2224,"%s/%s/%s",returnbuf,GFSCONFIGTAG,CONFTAG);

  lcp = open( TAGPATH, O_WRONLY|O_CREAT|O_TRUNC); 

  if (lcp == -1) 
  {
    print_connecterr();
    return -errno;
  }

  res = write(lcp, valuebuf, bufsize );
  if (res < 0 )
  {
     printf("ERR: failed to write value with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to write value with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;
}

int getjobqueue(char *returnbuf, size_t *returnbufsize)
{
  int res=0;
  int lcp;
  int idx=0;
  char mountpoint[2048] = { 0 };
  char TAGPATH[2224]= { 0 };
  char readbuf[262144]= { 0 };

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,2224,"%s/%s/queuelist",mountpoint,GFSCMDTAG);

  lcp = open( TAGPATH, O_RDWR|O_DIRECT ); 

  if (lcp == -1) 
  {
    print_connecterr();
    return -errno;
  }

  returnbuf[0]='0';
  res = pwrite(lcp, returnbuf,8 ,0 );
  if (res < 0 )
  {
     printf("ERR: failed to send empty buffer to initate server queue list with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }

  res=1;

  while (( res > 0 ) )
  {
    res = pread(lcp,readbuf,65536,idx);

    if ( res < 0 )
    {
       printf("ERR: failed to read queue request response with errno %d - %s\n",errno,strerror(errno));
       return -errno;
    }

    if ( res > 0 )
    {
      if ( readbuf[strlen(readbuf)-1] == '+' )
        readbuf[strlen(readbuf)-1]=0; 
      else if ( strlen(readbuf) < 65536 )
        res =0;

      idx+=snprintf(returnbuf+idx,*returnbufsize-idx-1,"%s",readbuf);
    }
  }

  *returnbufsize=idx;

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: requested queue action failed with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;
}

int jobqueue(int argc, char *argv[])
{
  int res=0;
  size_t retbufsize=1024000;
  size_t idx=0;
  int bytesprocessed=0;
  char returnbuf[1024000] = { 0 };

  struct QueueEntry {
    char jobid[8];
    char qslot[8];
    char state[32];
    char owner[8];
    char task[32];
  } QueueEntry;

  if ( argc < 3 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;    
  }

  if ( (strcmp(argv[2],"list") == 0) && ( argc == 3 ) )
  {
    if ( (res=getjobqueue(returnbuf,&retbufsize)) != 0 ) return res;
  }
  else if ( argc < 4 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;    
  }
  else if ( strcmp(argv[3],"jobid") == 0 )
  {
    if ( argc < 5 )
    {
      printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
      return -1;    
    }

    if ( (res=getjobqueue(returnbuf,&retbufsize)) != 0 ) return res;

    while ( idx < retbufsize )
    {
      if ( sscanf(returnbuf+idx,"%s %s %s %s %s %n", QueueEntry.jobid, QueueEntry.qslot, QueueEntry.state, QueueEntry.owner, QueueEntry.task, &bytesprocessed ) < 5 )
      {
        printf("\nNo Job found matching id %s\n",argv[4]);
        return 1;
      }

      if ( strcmp(QueueEntry.jobid,argv[4]) == 0 )
      {
        printf("Jobid: %s\nQueue Slot: %s\nJob State: %s\nJob Owner: %s\nJob Task: %s\n", QueueEntry.jobid, QueueEntry.qslot, QueueEntry.state, QueueEntry.owner, QueueEntry.task);
        return 0;
      }

      idx+=bytesprocessed;
    }
    printf("\nNo Job found matching id %s\n",argv[4]);
    return 1;
  }
  else
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;    
  }
  
  printf("\n%s",returnbuf);

  return 0;
}

int dumpconfig(int whichconfig)
{
  int loop=0;
  int res=0;
  char returnbuf[2048];

  for (loop=0; loop<GFSCONFIGTAGSCOUNT; loop++)
  {
    if ( whichconfig == 0 ) // configfile=0; liveconfig={other int}
    {
      if ( (res=gfs_getconfig(GFSVALUE,GFSCONFIGTAGS[loop],returnbuf,0)) != 0 ) return res;
    }
    else
    {
      if (( strcmp(GFSCONFIGTAGS[loop],"NEXFSCMD") != 0 )  && ( strcmp(GFSCONFIGTAGS[loop],"ROOTONLYACCESS") != 0 )&&  ( strcmp(GFSCONFIGTAGS[loop],"DEFAULTPERMISSIONS")   )) // NEXFSCMD is not a liveconfig
        if ( (res=getliveconfig(GFSCONFIGTAGS[loop],returnbuf,2048)) < 0 ) return res;
    }
    printf("%s:%s\n",GFSCONFIGTAGS[loop],returnbuf);
  }
  
  return 0;
}

int liveconfig(int argc, char *argv[])
{
  int res=0;
  char returnbuf[2048];

  if ( argc < 3 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;    
  }

  if ( strcmp(argv[2],"dumpall") == 0 )
  {
    return dumpconfig(1);
  }

  if ( argc < 4 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;    
  }

  if ( strcmp(argv[2],"get") == 0 )
  {
    if ( strcmp(argv[3],"value") == 0 )
    {
      if ( argc != 5 )
      {
        printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
        return -1;    
      }

      res=getliveconfig(argv[4],returnbuf,2047);

      if ( res < 0 )
      {
        printf("%s: Failed to retrieve live configuration data, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[3],"loglevel") == 0 )
    {
      res=getliveconfig("GFSLOGLEVEL",returnbuf,2047);
      if ( res < 0 )
      {
        printf("%s: Failed to retrieve live log level, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;

    }
    else if ( strcmp(argv[3],"syslogfacility") == 0 )
    {
      res=getliveconfig("SYSLOGFACILITY",returnbuf,2047);
      if ( res < 0 )
      {
        printf("%s: Failed to retrieve live syslog facility, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;

    }
  }
  else if ( strcmp(argv[2],"set") == 0 )
  {
    if ( strcmp(argv[3],"loglevel") == 0 )
    {
      res= setliveconfig("GFSLOGLEVEL",argv[4],strlen(argv[4]));

      if ( res == 0 )
      {
        printf("Updated Live Log Level\n");
      }
      return res;
    }
    else if ( strcmp(argv[3],"syslogfacility") == 0 )
    {
      res= setliveconfig("SYSLOGFACILITY",argv[4],strlen(argv[4]));

      if ( res == 0 )
      {
        printf("Updated Live Syslog Facility\n");
      }
      return res;
    }

    res=gfs_validateconfvalue(argv[3],argv[4],1);

    if ( res < 0 )
    {
      if ( res == -ENOTSUP )
      {
        printf("New value does not validate against configuration requirements\n");
        return ENOTSUP;
      }
      else if ( res == -EPERM )
      {
        printf("Configuration not live changable, restart required after updating config file to change\n");
        return EACCES;
      }
      else if ( res == -EACCES )
      printf("Access Denied, you do not have the required permissions required for this request\n");
      return res;
    }
    res= setliveconfig(argv[3],argv[4],strlen(argv[4]));

    if ( res == 0 )
    {
      printf("Updated Live Configuration\n");
    }
    return res;

  }

  printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
  return -1;
}

int ismounted()
{
  long fd;
  char buf[8192];
  char searchstr[4096];
  char mountpoint[2048];
  char *ptr;
  int res=1;

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(searchstr,4096,"nexfs.server %s ",mountpoint);

  fd = open("/proc/mounts",O_RDONLY);

  if ( fd == -1 )
  {
    printf("ERROR: Cannot open /proc/mounts, error - %s\n",strerror(errno)); 
    return -errno;
  }

  res=1;
  while ( res > 0 )
  {
    res=read(fd,buf,8192);

    if ( res < 0 )
    {
      printf("ERROR: cannot read from /proc/mounts, error - %s\n",strerror(errno)); 
      return -errno;
    }
    else if ( res != 0 )
    {
      ptr=strstr(buf,searchstr);

      if ( ptr != NULL )
      {
        return 1;
      }
    }
  }

  return 0;
}

int getserverstatus()
{
  char buf[65536] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  if ( ismounted() ==  0 )
  {
    printf("nexfs not running for mountpoint %s\n",mountpoint);
    return 0;
  }

  snprintf(TAGPATH,2224,"%s/%s/serverstatus",mountpoint,GFSCMDTAG);

  lcp = open( TAGPATH, O_RDWR|O_DIRECT ); 

  if (lcp == -1) 
  {
    if ( errno == 2 )
    {
      printf("Failed to connect to server,  nexfs is likely not running for mountpoint %s\n",mountpoint);
    }
    else
      printf("ERR: Failed to connect to a running nexfs server, errno %d - %s\n",errno,strerror(errno));

    return -errno;
  }

  res = pwrite(lcp, buf,65536 ,0 );
  if (res < 0 )
  {
     printf("ERR: failed to send emptyy buffer to initate server status with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = pread(lcp,buf,65536,0);

  if ( res < 0 )
  {
     printf("ERR: failed to read server status info with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }

  printf("Nexfs Mountpoint: %s\n", mountpoint);
  printf("%s",buf);

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request server status with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;
}

int checkdirectoryexists(char *CONFVAR, char *ENABLEDVAR)
{
  int res=0;
  char thedir[8192];
  char enabled[2];

  struct stat stbuf;

  if ( ENABLEDVAR != NULL )
  {
    res=gfs_getconfig(GFSVALUE,ENABLEDVAR,enabled,0);

    if ( res != 0 )
    {
      printf("ERROR: Cannot read config value %s\n",ENABLEDVAR);
      return -1;
    }

    if ( atoi(enabled) != 1 )
    {
      return 0;
    }
  }

  res=gfs_getconfig(GFSVALUE,CONFVAR,thedir,0);

  if ( res != 0 )
  {
    printf("ERROR: Cannot read config value %s\n",CONFVAR);
    return -1;
  }

  res=stat(thedir,&stbuf);

  if ( res != 0 )
  {
    printf("ERROR: cannto stat directory %s configured for %s \n",thedir, CONFVAR);
    return -1;
  }

  if ( (stbuf.st_mode & S_IFMT) != S_IFDIR )
  {
    printf("ERROR: value %s for configuration %s is not a directory \n",thedir, CONFVAR);
    return -1;
  }

  return 0;
}

int startserver()
{
  int res=0;
  char mountpoint[2048];
  char ALLOWOTHERS[2];
  char DEFAULTPERMISSIONS[2];
  char NEXFSCMD[2048];
  char CMD[4400];
  char LOCALCMD[4402];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"NEXFSCMD",NEXFSCMD,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs NEXFSCMD from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"ROOTONLYACCESS",ALLOWOTHERS,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs ROOTONLYACCESS from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"DEFAULTPERMISSIONS",DEFAULTPERMISSIONS,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs DEFAULTPERMISSIONS from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  if ( ismounted() == 1 )
  {
    printf("nexfs is registrered by the kernel as mounted for mountpoint %s\n",mountpoint);
    return -1;
  }

  if ( checkdirectoryexists("T1SDIR",NULL) != 0 ) return -1; 
  if ( checkdirectoryexists("T1DDIR","T1DDIRENABLED") != 0 ) return -1; 
  if ( checkdirectoryexists("T2DDIR","T2DDIRENABLED") != 0 ) return -1; 

  if ((  atoi(ALLOWOTHERS) == 0 ) && (atoi(DEFAULTPERMISSIONS) == 1 ))
  {
    snprintf(CMD,4400,"%s -o allow_other,default_permissions %s",NEXFSCMD,mountpoint);
  }
  else if ((  atoi(ALLOWOTHERS) == 0 ) && ( atoi(DEFAULTPERMISSIONS) == 0 ))
  {
    snprintf(CMD,4400,"%s -o allow_other %s",NEXFSCMD,mountpoint);
  }
  else if ((  atoi(ALLOWOTHERS) == 1 ) && ( atoi(DEFAULTPERMISSIONS) == 1 ))
  { 
    snprintf(CMD,4400,"%s -o default_permissions %s",NEXFSCMD,mountpoint);
  }
  else
  {
    snprintf(CMD,4400,"%s %s",NEXFSCMD,mountpoint);
  }

  snprintf(LOCALCMD,4402,"./%s",CMD);

  res=system(CMD);

  if ( res != 0 )
  {
    printf("nexfs.server not found in search PATH, trying to start nexfs.server from the current directory\n");
    res=system(LOCALCMD);

    if ( res != 0 )
    {
      printf("Failed to start nexfs.server, check configurations NEXFSCMD and MOUNTPOINT are set correct, currently set to %s and %s, error if given %s\n",NEXFSCMD,mountpoint,strerror(errno));
      return -1;
    }
  }

  if ( ismounted() == 1 )
  {
    printf("nexfs.server started, mounted to %s\n",mountpoint);
  }
  else
  {
    printf("ERROR: nexfs.server start sent, but not registered by kernel as mounted\n");
    return -1;
  }
    
  return 0;
}

int stopserver(int force)
{
  int res=0;
  int tries=0;
  int fd=0;
  char mountpoint[2048];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",res,strerror(errno));
    return -errno;
  }

  if ( ismounted() == 0 )
  {
    printf("nexfs not running for mountpoint %s\n",mountpoint);
    return -1;
  }

  if ( force == 1 )
  {
    res = umount2(mountpoint,MNT_DETACH);
    sleep(1);
    res = umount2(mountpoint,MNT_FORCE);
    printf("Sent hard shutdown request to Nexfs\n");
    return 0;
  }

  res= setliveconfig("NEXFSPAUSED","1",1);

  if ( res == 0 )
    res= setliveconfig("JOBSCHEDULARPAUSED","1",1);

  if ( res == 0 )
    res= setliveconfig("BGMIGRATIONSPAUSED","1",1);

  if ( res == 0 )
  {
    printf("Requested Pausing of Nexfs Operations\n");

    printf("Waiting for outstanding nexfs operatings to complete");
    while ( tries < 60 )
    {
      sleep(1);

      if ( ( fd=open(GFSNULLTAG,O_RDONLY)) == -1 )
      {
        if ( errno ==  2 )
          break;
      }
      close(fd);
      tries++;
      printf(".");
    }

    printf("\n");

    if ( tries == 60 )
      printf("Failed to paused Nexfs Operations, continuing shutdown\n");
  }
  else
  {
    printf("Failed to pause Nexfs Operations, continuing shutdown\n");
  }

  res = umount2(mountpoint,MNT_DETACH);

  printf("Sent shutdown request to Nexfs\n");
    
  return 0;
}

int openstructurefile(char *filename, uint64_t *sfp)
{
  int res;
  char returnbuf[2048];
  char SDIRFILE[8192];

  res=gfs_getconfig(GFSVALUE,"T1SDIR",returnbuf,0);

  if ( res < 0 )
  {
    printf("Failed to open file information, errno - %s\n",strerror(errno));
    return -errno;
  }

  snprintf(SDIRFILE,8191,"%s/1e/%s",returnbuf,filename);

  *sfp=open(SDIRFILE, O_RDONLY );

  if ( *sfp == -1 )
  {
    printf("ERROR: Cannot open structure file, error - %s\n",strerror(errno)); 
    return -errno;
  }

  return 0;
}

int displayfileinfo(char *filename, int option )
{
  char returnbuf[2048];
  int res;
  uint64_t sfp; 

  if ( ( res=openstructurefile(filename,&sfp) ) < 0 ) return -errno;

  if ( option == 1 ) 
  {
    if ( (res = fgetxattr(sfp, "user.gfsid", returnbuf, 37)) == -1 )
    {
      printf("ERROR: Failed to retrive UID, error - %s\n",strerror(errno));
      return -errno;
    }
    returnbuf[res]=0;
    printf("FID: %s\n", returnbuf);
  }

  if ( (res = fgetxattr(sfp, "user.datapartsize", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive datapartsize, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Datapartsize: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.mintiered", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive mintiered, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Mintiered: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.maxtiered", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive maxtiered, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Maxtiered: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.lockedtotier", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive lockedtotier, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("LockToTier: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.oldesttier1datafile", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive oldesttier1datafile, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Oldesttier1datafile: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.oldesttier2datafile", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive oldesttier2datafile, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Oldesttier2datafile: %s\n", returnbuf);


  if ( (res = fgetxattr(sfp, "user.structureversion", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive structureversion, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Structureversion: %s\n", returnbuf);

  close(sfp); 
  return 0;
}

int setfileinfo(char *filename, char *attrib, char *newvalue)
{
  int res;
  uint64_t sfp; 

  if ( ( res=openstructurefile(filename,&sfp) ) < 0 ) return -errno;

  if ( (res = fsetxattr(sfp, attrib, newvalue, strlen(newvalue),XATTR_REPLACE)) == -1 )
  {
    printf("ERROR: Failed to set file info, error - %s\n",strerror(errno));
    return -errno;
  }

  printf("File info changed\n");
  return 0;
}

int file (int argc, char *argv[])
{
  if ( argc < 4 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[2],"info") == 0 )
  {
    if ( argc == 5 )
    {
      if ( strcmp(argv[4],"-fid") == 0)
      {
        return displayfileinfo(argv[3],1);
      }
    }
    else if ( argc == 7 )
    {
      if ( strcmp(argv[3],"-set") == 0)
      {
        return setfileinfo(argv[4],argv[5],argv[6]);
      } 
    }
    else
    {
      return displayfileinfo(argv[3],0);
    }
  }
  else if ( strcmp(argv[2],"tierusage") == 0 )
  {
    return showfiletierusage(argv[3],0);
  }
  else if ( strcmp(argv[2],"extendedtierusage") == 0)
  {
    return showfiletierusage(argv[3],1);
  }
  else if ( strcmp(argv[2],"movetotier3") == 0 )
  {
    return movetotier(3,argv[3]);
  }
  printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
  return -1;
}

int configfiles_setdefaults(int option)
{
  int defrows=10;
  int loop=0;
  int res=0;
  char newvalue[32] = { 0 };
  char confirm[4];

  char *DEFAULTS[][3][32] = { 
    { {"T3USEHTTPS"}, {"0"}, {"1" }},
    { {"T3S3USEVIRTUALHOST"}, {"0"}, {"1"}},
    { {"T3S3SIGNATUREV4"}, {"1"}, {"1"}},
    { {"T3S3ENABLEREGION"}, {"0"}, {"1"}},
    { {"T3S3COMPATIBLE"}, {"1"}, {"0"}},
    { {"T3S3URL"}, {"localhost"}, {"amazonaws.com"}},
    { {"T3S3PORT"}, {"9000"}, {"443"}},
    { {"T3S3RETRYSLEEP"}, {"1"}, {"1"}},
    { {"T3S3RETRIES"}, {"3"}, {"3"}},
    { {"T3S3RETRY404"}, {"0"}, {"0"}}
  };

  char MSG[2][372] = {
    "\nDefaults loaded, please run and set the following\nnexfscli configfile set T3S3URL {minio host}\nnexfscli configfile set T3S3PORT {minio port}\nnexfscli configfile set T3S3BUCKET {bucket}\nnexfscli configfile set T3AWSAccessKeyId {Access Key}\nnexfscli configfile set T3AWSSecretAccessKey {secret access key}\n",
    "\nDefaults loaded, please run and set the following\nnexfscli configile set T3S3REGION {AWS Region}\nnexfscli configfile set T3S3BUCKET {S3 bucket}\nnexfscli configfile set T3AWSAccessKeyId {S3 Access Key}\nnexfscli configfile set T3AWSSecretAccessKey {S3 secret access key}\n"
  };

  printf("\nPlease confirm overwrite of config file parms with requested defaults (y/n): ");

  fgets(confirm,3,stdin);

  if ( (strncmp(confirm,"y",1) == 0 ) || ( strncmp(confirm,"Y",1) == 0 ) || ( strncmp(confirm,"yes",3) == 0) || ( strncmp(confirm,"YES",3) == 0 ))
  {
    for ( loop=0; loop < defrows; loop++ )
    {
      if ( option == 0 ) // 0=minio, 1=AWS S3
      {
        strcpy(newvalue,*DEFAULTS[loop][1]);
      }
      else
      {
        strcpy(newvalue,*DEFAULTS[loop][2]);
      }
    
      res=gfs_updateconfigfile(GFSVALUE,*DEFAULTS[loop][0],newvalue,strlen(newvalue));

      if ( res == 0 )
      {
        printf("Updated value of conf tag '%s' to '%s'\n",*DEFAULTS[loop][0],newvalue);
      }
      else
      {
        printf("%s: Failed to update configuration data, errno %d - %s\n",*DEFAULTS[loop][0],res,strerror(errno));
        return -1;
      }
    }

    if ( option == 0 ) // 0=minio, 1=AWS S3
      printf("%s",MSG[0]);
    else
      printf("%s",MSG[1]);

    printf("Restart nexfs server for new configuration to take effect\n");
    
    return 0;
  }
  printf("\nDefaults not set\n");
  return -1;
}

int configfiles(int argc, char *argv[])
{
  int res=0;
  char returnbuf[2048];
  char LOGLEVELTAG[] = "GFSLOGLEVEL";

  if ( argc < 3 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[2],"dumpall") == 0 )
  {
    return dumpconfig(0);
  }

  if ( argc != 5 ) 
  {
    if ( ( argc != 4 ) && ( strcmp(argv[3],"loglevel") != 0 ))
    {
      printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
      return -1;
    }
  }

  if ( strcmp(argv[2],"set") == 0 )
  {
    if ( strcmp(argv[3],"defaults") == 0 )
    {
      if ( strcmp(argv[4],"minio") == 0  )
        return configfiles_setdefaults(0);
      else if ( strcmp(argv[4],"s3") == 0  )
        return configfiles_setdefaults(1);

      printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
      return -1;
    }

    if ( strcmp(argv[3],"loglevel") == 0 )
      argv[3]=LOGLEVELTAG;

    res=gfs_validateconfvalue(argv[3],argv[4],0);

    if ( res < 0 )
    {
      printf("Failed to validate that new value meets configuration requirements\n");
      return -1;
    }

    if ( res == -ENOTSUP )
    {
      printf("New value does not validate against configuration requirements\n");
      return ENOTSUP;
    }

    res=gfs_updateconfigfile(GFSVALUE,argv[3],argv[4],strlen(argv[4]));

    if ( res == 0 )
    {
      printf("Updated Configuration file\n");
      return 0;
    }

    printf("%s: Failed to update configuration data, errno %d - %s\n",argv[0],errno,strerror(errno));

    return -errno;
  }
  else if ( strcmp(argv[2],"get") == 0 )
  {
    if ( strcmp(argv[3],"taglabel") == 0 )
    {
      res=gfs_getconfig(GFSTAG,argv[4],returnbuf,0);

      if ( res == -1 )
      {
        printf("%s: Failed to retrieve configuration data, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[3],"loglevel") == 0 )
    {
      res=gfs_getconfig(GFSVALUE,"GFSLOGLEVEL",returnbuf,2047);
      if ( res < 0 )
      {
        printf("%s: Failed to retrieve live log level, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[3],"value") == 0 )
    {
      res=gfs_getconfig(GFSVALUE,argv[4],returnbuf,0);

      if ( res < 0 )
      {
        printf("%s: Failed to retrieve configuration data, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[3],"help") == 0 )
    {
      res=gfs_getconfig(GFSHELP,argv[4],returnbuf,0);

      if ( res == -1 )
      {
        printf("%s: Failed to retrieve configuration data, errno %d - %s\n",argv[0],errno,strerror(errno));
        return -errno;
      }
      printf("%s\n",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[3],"all") == 0 )
    {
      res=gfs_getconfig(GFSTAG,argv[4],returnbuf,0);

      if ( res > -1 )
      {
        printf("Tag Label = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVALUE,argv[4],returnbuf,0);
        printf("Value = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSHELP,argv[4],returnbuf,0);
        printf("Help Text = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVSTRING,argv[4],returnbuf,0);
        printf("Validation:String = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVMIN,argv[4],returnbuf,0);
        printf("Validation:Min(Value/StringLength) = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVMAX,argv[4],returnbuf,0);
        printf("Validation:Max(Value/StringLength) = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSRESTART,argv[4],returnbuf,0);
        printf("Requires Restart = '%s'\n",returnbuf);

        return 0;
      }

      printf("%s: Failed to retrieve configuration data, errno %d - %s\n",argv[0],errno,strerror(errno));
      return -errno;
    }
  }
  printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
  return -1;
}

int manageadmingroup(char *argv[])
{
  int groupid=0;
  int res=0;
  int loop=0;
  int stringvalue=0;
  char gid[10] = { 0 };
  char CONFIGVAR[32]= { 0 }; 
  // char CONFIGVARPATH[32]= { 0 }; 
  char buffer[10] = { 0 };
  struct group *groupinfo;

  if ( strcmp(argv[4],"cmd") == 0 ) 
  {
    strcpy(CONFIGVAR,"SECURITYCMDGROUP");
    // strcpy(CONFIGVARPATH,"NEXFSAPICHANGECMDGROUP");
  }
  else if ( strcmp(argv[4],"config") == 0 ) 
  {
    strcpy(CONFIGVAR,"SECURITYCONFGROUP");
    // strcpy(CONFIGVARPATH,"NEXFSAPICHANGECONFGROUP");
  }
  else
  {
    printf("%s: Unknown command format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[3],"get") == 0 ) 
  {
    res=getliveconfig(CONFIGVAR,buffer,10);
    
    if ( res < 0 )
    {
      printf("Cannot load liveconfig, attempting to load from configuration file\n");
      res=gfs_getconfig(GFSVALUE,CONFIGVAR,buffer,0);
    }
    
    if ( res < 0 )
    {
      printf("Failed to retrieve nexfs %s from configuration data, errno %d - %s\n",CONFIGVAR,errno,strerror(errno));
      return -errno;
    }

    groupid=atoi(buffer);
    groupinfo=getgrgid(groupid);  // group defaults to 0(root) if no otherwise a valid number

    if ( groupinfo == NULL )
    {
      printf("Failed to retrieve group name for group id %d, errno %d - %s\n",groupid,errno,strerror(errno));
      return -errno;
    }
    
    printf("group id %d: group name: %s\n",groupid,groupinfo->gr_name);
    return 0;
  }
  else if ( strcmp(argv[3],"set") == 0 ) 
  {
    for ( loop=0; loop < strlen(argv[6]); loop++ )
    {
      if ( ! isdigit (argv[6][loop])) stringvalue=1; 
    }

    if ( stringvalue == 1 )
    {
      groupinfo=getgrnam(argv[6]);  

      if ( groupinfo == NULL )
      {
        if ( errno == 0 )
          printf("Failed to retrieve system groupid from OS for group name '%s'\n",argv[6]);
        else
          printf("Failed to retrieve groupid for group name '%s', errno %d - %s\n",argv[6],errno,strerror(errno));
        return -errno;
      }

      snprintf(gid,10,"%d",groupinfo->gr_gid);
      groupid=groupinfo->gr_gid;
    }
    else
    {
      snprintf(gid,10,"%s",argv[6]);
      groupid=atoi(argv[6]);
    }

    if ( gfs_updateconfigfile(GFSVALUE,CONFIGVAR,gid,strlen(gid)) != 0 )
    {
      printf("Failed to set configfile groupid, errno %d - %s\n",errno,strerror(errno));
      return -errno;
    }
    else
      printf("Set configfile %s admin groupid to %s\n",argv[4],gid);

    if ( setliveconfig(CONFIGVAR,gid, strlen(gid)) != 0 )
    {
      printf("Failed to set liveconfig groupid, errno %d - %s\n",errno,strerror(errno));
      return -errno;
    }
    else
      printf("Set liveconfig %s admin groupid to %s\n",argv[4],gid);

    return 0;
  }

  return -1;
}

int manageadminaccess(char *argv[])
{
    int accessrights=0;
  int res=0;
  char access[10] = { 0 };
  char CONFIGVAR[32]= { 0 }; 
  char buffer[10] = { 0 };


  if ( strcmp(argv[4],"cmd") == 0 ) 
  {
    strcpy(CONFIGVAR,"SECURITYCMDGROUPACCESS");
  }
  else if ( strcmp(argv[4],"config") == 0 ) 
  {
    strcpy(CONFIGVAR,"SECURITYCONFGROUPACCESS");
  }
  else
  {
    printf("%s: Unknown command format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[3],"get") == 0 ) 
  {
    res=getliveconfig(CONFIGVAR,buffer,10);
    
    if ( res < 0 )
    {
      printf("Cannot load liveconfig, attempting to load from configuration file\n");
      res=gfs_getconfig(GFSVALUE,CONFIGVAR,buffer,0);
    }
    
    if ( res < 0 )
    {
      printf("Failed to retrieve nexfs %s from configuration data, errno %d - %s\n",CONFIGVAR,errno,strerror(errno));
      return -errno;
    }

    accessrights=atoi(buffer);
    if ( accessrights == 2 )
      strcpy(access,"RW");
    else if ( accessrights == 1 )
      strcpy(access,"R-");
    else
      strcpy(access,"--");
    
    printf("%d (%s)\n",accessrights,access);

    return 0;
  }
  else if ( strcmp(argv[3],"set") == 0 ) 
  {
    printf("Function not currently implemented\n");

    return 0;
  }

  return -1;
}

int security(int argc, char *argv[])
{
  if ( argc != 7 )
  {
    if ( argc == 6 )
    {
      if ( strcmp(argv[3],"get") != 0 )
      {
        printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
        return -1;
      }
    }
    else
    {
      printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
      return -1;
    }
  } 

  if (( strcmp(argv[2],"admin") == 0 ) &&  ( strcmp(argv[5],"group") == 0 ))
  {
    return ( manageadmingroup(argv) );
  }
  if (( strcmp(argv[2],"admin") == 0 ) &&  ( strcmp(argv[5],"access") == 0 ))
  {
    return ( manageadminaccess(argv) );
  }

  printf("%s: Unknown command format, type %s help\n",argv[0],argv[0]);

  return -1;
}
int server(int argc, char *argv[])
{
  if ( argc != 3 )
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[2],"start") == 0 )
  {
    return ( startserver() );
  }
  else if ( strcmp(argv[2],"status") == 0 )
  {
    return ( getserverstatus() );
  }
  else if ( strcmp(argv[2],"stop") == 0 )
  {
    return ( stopserver(0) );
  }
  else if ( strcmp(argv[2],"forcestop") == 0 )
  {
    return ( stopserver(1) );
  }
  else if ( strcmp(argv[2],"stats") == 0 )
  {
    return ( getserverstats() );
  }

  printf("%s: Unknown command format, type %s help\n",argv[0],argv[0]);

  return -1;
} 

int init(int argc, char *argv[])
{
  int res=0;
  struct stat stbuf = {0};

  res = stat(DEFAULTGFSCONFDIR,&stbuf);

  if ( res != 0 && errno != ENOENT )
  {
    printf("%s: Failed to stat configuration directory %s, errno %d - %s\n",argv[0],DEFAULTGFSCONFDIR,errno,strerror(errno));
    return -errno; 
  }

  if ( errno == ENOENT )
  {
    res = mkdir(DEFAULTGFSCONFDIR, 0755);

    if ( res != 0 )
    {
      printf("%s: Failed to create configuration directory %s, errno %d - %s\n",argv[0],DEFAULTGFSCONFDIR,errno,strerror(errno));
      return -errno;
    }
  }

  res=generateconfigfiles(argv);

  return res;
}

int setupdatastores(int argc, char *argv[])
{
  int res=0;
  int loop=0;
  int loop1=0;
  int loop2=0;
  int loop3=0;
  int T1DDIRENABLED;
  int T2DDIRENABLED;
  char T1DDIR[256000]= {0};
  char T2DDIR[256000]= {0};
  char basefoldername[256016]= { 0 };
  char foldername[256064]= { 0 };
  char buffer[10] = { 0 };

  res=gfs_getconfig(GFSVALUE,"T1SDIR",buffer,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T1SDIR from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }
  
  snprintf(basefoldername,260016,"%s/1e",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }

  snprintf(basefoldername,260016,"%s/de",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }

  snprintf(basefoldername,260016,"%s/df",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }


  res=gfs_getconfig(GFSVALUE,"T1DDIRENABLED",buffer,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T1DDIRENABLED from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  T1DDIRENABLED=atoi(buffer);

  res=gfs_getconfig(GFSVALUE,"T2DDIRENABLED",buffer,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T2DDIRENABLED from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  T2DDIRENABLED=atoi(buffer);

  if ( T1DDIRENABLED == 1 )
  {
    res=gfs_getconfig(GFSVALUE,"T1DDIR",T1DDIR,0);

    if ( res == -1 )
    {
      printf("Failed to retrieve nexfs T1DDIR from configuration data, errno %d - %s\n",errno,strerror(errno));
      return -errno;
    }
  }

  if ( T2DDIRENABLED == 1 )
  {
    res=gfs_getconfig(GFSVALUE,"T2DDIR",T2DDIR,0);

    if ( res == -1 )
    {
      printf("Failed to retrieve nexfs T2DDIR from configuration data, errno %d - %s\n",errno,strerror(errno));
      return -errno;
    }
  }

  for (loop=0; loop<256; loop++)
  {
    printf("\r%d/255",loop);
    fflush(stdout); 

    for (loop1=0; loop1 <2; loop1++) 
    {
      basefoldername[0]=0;
      if (( loop1 == 0 ) && ( T1DDIRENABLED == 1 ))
        snprintf(basefoldername,260016,"%s/%02x",T1DDIR,loop);
      else if (( loop1 == 1 ) && ( T2DDIRENABLED == 1 ))
        snprintf(basefoldername,260016,"%s/%02x",T2DDIR,loop);

      if ( basefoldername[0] == 0 )
        continue;

      res = mkdir(basefoldername, 0755);

      if (( res != 0 ) && ( errno != EEXIST ))
      {
        printf("%s: Failed to create data directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
        return -errno;
      }

      for (loop2=0; loop2<256; loop2++)
      {
        snprintf(foldername,256025,"%s/%02x",basefoldername,loop2);

        res = mkdir(foldername, 0755);

        if (( res != 0 ) && ( errno != EEXIST ))
        {
          printf("%s: Failed to create data directory %s, errno %d - %s\n",argv[0],foldername,errno,strerror(errno));
          return -errno;
        }

        for (loop3=0; loop3<16; loop3++)
        {
          snprintf(foldername,256063,"%s/%02x/%0X",basefoldername,loop2,loop3);

          res = mkdir(foldername, 0755);

          if (( res != 0 ) && ( errno != EEXIST ))
          {
            printf("%s: Failed to create data directory %s, errno %d - %s\n",argv[0],foldername,errno,strerror(errno));
            return -errno;
          }
        }
      }
    }  
  }

  return 0;
}

int printconfigtag(char *tagname)
{
  char *argv[]= { 0,0,"get","all",tagname };

  return configfiles(5, argv);
}

int help(int argc, char *argv[])
{
  if ( argc > 2 )
  {
    char returnbuf[2048];

    if ( strcmp(argv[2],"loglevels") == 0 )
    {
      gfs_printloglevels(returnbuf,2048);
      printf("Valid Loglevels:\n%s",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[2],"syslogfacility") == 0 )
    {
      gfs_printsyslogfacilities(returnbuf,2048);
      printf("Valid syslogfacilities:\n%s",returnbuf);
      return 0;
    }
    else if ( strcmp(argv[2],"configtags") == 0 )
    {
      printconfigtags();
      return 0;
    }
    else if ( argc == 4 )
    {
      if ( strcmp(argv[2],"configtag") == 0 )
      {
        printconfigtag(argv[3]);
        return 0;
      }
    }
  }

  printf("configfile get all [tagname]\n");
  printf("configfile get taglabel [tagname]\n");
  printf("configfile get value [tagname]\n");
  printf("configfile get help [tagname]\n");
  printf("configfile set [tagname] [newvalue]\n");
  printf("configfile set defaults [system]  - where system is either minio or s3\n");
  printf("configfile get loglevel\n");
  printf("configfile dumpall\n");
  printf("liveconfig get value [tagname]\n");
  printf("liveconfig set [tagname] [newvalue]\n");
  printf("liveconfig get loglevel\n");
  printf("liveconfig set loglevel [newloglevel]\n");
  printf("liveconfig get syslogfacility\n");
  printf("liveconfig set syslogfacility [newsyslogfacility]\n");
  printf("liveconfig dumpall\n");
  printf("security admin get cmd group\n");
  printf("security admin set cmd group [groupname or id]\n");
  printf("security admin get config group\n");
  printf("security admin set config group [groupname or id]\n");
  printf("security admin get cmd access\n");
  printf("security admin get config access\n");
  printf("help loglevels\n");
  printf("help syslogfacility\n");
  printf("help configtags\n");
  printf("help configtag [tagname]\n");
  printf("release get nexfscli\n");
  printf("release get nexfs\n");
  printf("file info [filename (including nexfs path)]\n");
  printf("file tierusage [filename (including nexfs path)]\n");
  printf("file extendedtierusage [filename (including nexfs path)]\n");
  printf("file movetotier3 [filename (including path)]\n");
  printf("jobqueue list\n");
  printf("jobqueue list jobid [jobid]\n");
  printf("server status\n");
  printf("server start\n");
  printf("server stop\n");
  printf("server forcestop\n");
  printf("server stats\n");
  return 0;
}

int allhelp(int argc, char *argv[])
{
  printf("-init\n");
  printf("setupdatastores\n");
  printf("generateconfigfiles\n");
  printf("file info -set [filename (including nexfs path)] attrib newvalue\n");

  help(argc,argv);

  return 0;
}

int main(int argc, char *argv[])
{
  int res=0;

  if ( argc == 1 )
  { 
    printf("%s: unknown request, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  if ( strcmp(argv[1],"-allhelp") == 0 )
  {
    allhelp(argc, argv);
    return 0;
  } 
  else if ( strcmp(argv[1],"help") == 0 )
  {
    help(argc, argv);
    return 0;
  }
  else if ( strcmp(argv[1],"generateconfigfiles") == 0 )
  {
    res=generateconfigfiles(argv);
    return res;
  }
  else if ( strcmp(argv[1],"configfile") == 0 )
  {
    res=configfiles(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"liveconfig") == 0 )
  {
    res=liveconfig(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"release") == 0 )
  {
    res=releaseinfo(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"file") == 0 )
  {
    res=file(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"-init") == 0 )
  {
    res=init(argc,argv);
    return res;
  }
  else if ( strcmp(argv[1],"setupdatastores") == 0 )
  {
    res=setupdatastores(argc,argv);
    return res;
  }
  else if ( strcmp(argv[1],"jobqueue") == 0 )
  {
    res=jobqueue(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"server") == 0 )
  {
    res=server(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"security") == 0 )
  {
    res=security(argc, argv);
    return res;
  }
  else
  {
    printf("%s: unknown request, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  errno=res*-1;  
  return res;
}
