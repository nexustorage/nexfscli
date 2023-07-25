// Copyright (c) 2021-2023 Nexustorage Limited.
// Copyright (c) 2021-2023 Glen Olsen (glen @ glenolsen.net).
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
#include <dirent.h>
#include <grp.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "gfsconf_common.h"

//#include "gfsconf_funcs.h"
// #include "gfslogging.h"

#define NEXFSCLIRELEASE "1.01(23)"
#define QUEUELIST 1 
#define NEXFSCLI 1 

struct struct_switches {
  int upgrade;
  int reinstall;
  int configoverwrite;
  int nosoftwareinstall;
  int noserviceinstall; 
  int noinit;
  int generateconfigfiles;
  int nonfs;
  int noiscsi;
  int accepttermsandcondition;
  int noinstallpackages;
  int noprompt;
  int nocheckbinaries;

} cmdlineswitches = { 0 };

char *MYNAME;
const int EQUAL=0;
const int GREATER=1;
int slient=0;
// a few "ghost" functions that need to be defined to link against gfsconf.o
void gfs_startnfssystem()
{
  return;
}
void gfs_stopnfssystem(void)
{
  return;
}
void gfs_createnfsexportsymlink()
{
  return;
}

void gfs_check_server_functionaility()
{
  return;
}

void gfs_setstructrep()
{
  return;
}

void gfs_clearlicense(void)
{
  return;
}
// end of ghost functions

int process_argvswitches(int *argc, char *argv[] )
{
  int loop=0;

  for( loop=1; loop < *argc; loop++ )
  {
    if ( strncmp(argv[loop],"--",2) == 0 )
    {
      switch (gfs_configkeynamesum(argv[loop]))
      {
        case 4521: // --upgrade
          cmdlineswitches.upgrade=1;
          break;

        case 6940: // --reinstall
          cmdlineswitches.reinstall=1;
          break;

        case 20430: // --nosoftwareinstall
          cmdlineswitches.nosoftwareinstall=1;
          break;

        case 18250: // --noserviceinstall
          cmdlineswitches.noserviceinstall=1;
          break;

        case 3757: // --noinit
          cmdlineswitches.noinit=1;
          break;

        case 29027: // --generateconfigfiles 
          cmdlineswitches.generateconfigfiles=1;
          break;

        case 2876: // --nonfs 
          cmdlineswitches.nonfs=1;
          break;

        /* case 4682: // --noiscsi 
          cmdlineswitches.noiscsi=1;
          break; */

        case 34711: // --accepttermsandcondition 
          cmdlineswitches.accepttermsandcondition=1;
          break;

        case 19870: // --noinstallpackages 
          cmdlineswitches.noinstallpackages=1;
          break;

        case 5970: // --noprompt 
          cmdlineswitches.noprompt=1;
          break;

        case 15948: // --nocheckbinaries 
          cmdlineswitches.nocheckbinaries=1;
          break;

        default:
          if ( strncmp(argv[loop],"--confdir=",10) == 0 )
          {
            GFSCONFDIR=strstr(argv[loop],"=")+1;
            --*argc;
          }
          else
          { 
            printf("ERR: unknown commandline option passed %s\n",argv[loop]);
            return -1; 
          }
      }

    }
  }

  return 0;
}
void print_connecterr()
{
  char mountpoint[2048] = { 0 };
  gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);
  printf("Connection to nexfs server failed over mountpoint (%s), check any passed options are correct or server status by running: nexfscli server status\n",mountpoint);
}

int generateconfigfiles()
{
  int res=0;
  char basefoldername[260020];

  printf("Generating Configuration Files\n");

  
  res=gfs_loadconfig_common(1);
 
  if ( res == 0 )
  {
    snprintf(basefoldername,260016,"%s/%s",GFSCONFDIR,GFSCONFIGTAG);
    res = mkdir(basefoldername, 0700);
    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create config directory %s, errno %d - %s\n",MYNAME,basefoldername,errno,strerror(errno));
      return -errno;
    }

    if ( errno != EEXIST )
    {
      res=chown(basefoldername, 0, 0);
      if ( res != 0 )
      {
        printf("%s: Failed to chown config directory %s, errno %d - %s\n",MYNAME,basefoldername,errno,strerror(errno));
        return -errno;
      }
    }

    errno=0;
    snprintf(basefoldername,260016,"%s/%s",GFSCONFDIR,GFSCMDTAG);
    res = mkdir(basefoldername, 0700);

    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create config directory %s, errno %d - %s\n",MYNAME,basefoldername,errno,strerror(errno));
      return -errno;
    }

    if ( errno != EEXIST )
    {
      res=chown(basefoldername, 0, 0);
      if ( res != 0 )  
      {
        printf("%s: Failed to chown config directory %s, errno %d - %s\n",MYNAME,basefoldername,errno,strerror(errno));
        return -errno;
      }
    }

    printf("Done.\n");
  }
  else
  {
    printf(".. Returned %d - FAILED, hint: does /etc/nexfs exist, have you run nexfscli init?\n",res);
    return -1;
  }

  return 0;
} 

int gfs_licensedetails(char * x, ...) // needed to link against gfsconf.o
{
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

int check_args(int args, int expected, int equalgreater )
{
  if ((( equalgreater == EQUAL ) && ( args != expected)) || ((equalgreater == GREATER) && ( args < expected )))
  {
    printf("%s: incorrect format, type %s help\n",MYNAME,MYNAME);
    return 1;
  }
  return 0;
}



enum pkgmgr {
  apt=1,
  rpm
};

int whichpackagemanager()
{
  struct stat stbuf;

  if (( stat("/usr/bin/apt-get",&stbuf) == 0 ) && ( stat("/usr/bin/dpkg-query",&stbuf) == 0 ))
    return apt;

  if (( stat("/usr/bin/yum",&stbuf) == 0 ) && ( stat("/usr/bin/rpm",&stbuf) == 0 ))
    return rpm;
  
  return 0;
}

int installaptpackagedepnds()
{
  const char * aptpackages[] =  {"fuse3", "libcurl4", "libuuid1", "openssl", "libssl-dev"};
  char cmd[1024] = { 0 };
  const int packages = 4;
  int package=0;
  int res=0;

  for ( package=0; package < packages; package++ )
  {
    snprintf(cmd,1024,"/usr/bin/dpkg-query  -Wf'${db:Status-abbrev}' %s",aptpackages[package]); 

    res = system(cmd);

    if ( res == -1  )
    {
      printf("ERR: failed to run cmd %s : errno %s ",cmd,strerror(errno));
      return -1;
    }
    else if ( WEXITSTATUS(res) == 1 )
    {
      snprintf(cmd,1024,"/usr/bin/apt-get install -y %s",aptpackages[package]);
      res = system(cmd);

      if ( res != 0 )      
      {
        printf("ERR: failed to run cmd %s : errno %s ",cmd,strerror(errno));
        return -1;
      }
    }
  }
  return 0;
}

int installrpmpackagedepends()
{
  const char * rpmpackages[] =  {"fuse3", "compat-openssl11", "libcurl", "libuuid","nfs-utils"};
  char cmd[1024] = { 0 };
  const int packages = 4;
  int package=0;
  int res=0;

  for ( package=0; package < packages; package++ )
  {
    if (( strcmp(rpmpackages[package],"nfs-utils") == 0 ) && ( cmdlineswitches.nonfs == 1 ))
      continue;

    snprintf(cmd,1024,"/usr/bin/rpm -q %s",rpmpackages[package]); 

    res = system(cmd);

    if ( res == -1  )
    {
      printf("ERR: failed to run cmd %s : errno %s ",cmd,strerror(errno));
      return -1;
    }
    else if ( WEXITSTATUS(res) == 1 )
    {
      snprintf(cmd,1024,"/usr/bin/yum -y install %s",rpmpackages[package]);
      res = system(cmd);

      if ( res != 0 )      
      {
        printf("ERR: cmd %s failed (or package could not be installed): errno %s ",cmd,strerror(errno));
        return -1;
      }
    }
  }
  return 0;
}

int downloadinstallnexfsbinaries()
{
  const char * nexfsurl = "https://github.com/nexustorage/Nexfs-Public-Download/raw/main/nexfs.server";
  const char * nexfscliurl = "https://github.com/nexustorage/Nexfs-Public-Download/raw/main/nexfscli";

  const char *curlgetnexfs = "/usr/bin/curl -L -o /usr/sbin/nexfs.server";
  const char *wgetnexfs = "/usr/bin/wget -O /usr/sbin/nexfs.server";
  const char *curlgetnexfscli = "/usr/bin/curl -L -o /usr/bin/nexfscli";
  const char *wgetnexfscli = "/usr/bin/wget -O /usr/bin/nexfscli";

  char downloadcmd[2048] = { 0 };
  int res=0;
  struct stat stbuf;

  if (( stat("/usr/sbin/nexfs.server",&stbuf) == 0 ) && (( cmdlineswitches.reinstall == 0 ) && ( cmdlineswitches.upgrade == 0)))
  {
      printf("ERR: /usr/sbin/nexfs.server already exists and upgrade or reinstall not specified\n");
      return -1;
  }

  snprintf(downloadcmd,2048,"%s %s 2>&1 | grep -q 'HTTP/2 200'",curlgetnexfs,nexfsurl);
  res=system(downloadcmd);

  if (( res == -1 ) || ( res == 256 )) 
  {
    snprintf(downloadcmd,2048,"%s %s",wgetnexfs,nexfsurl);
    res=system(downloadcmd);

    if (( res == -1 ) || ( res == 256 ))
    {
      printf("ERR: failed to download latest release of nexfs.server, error status %d\n",res);
      return -1;
    }
  }

  if ( WEXITSTATUS(res)  != 0 )
  {
    printf("ERR: failed to download latest release of nexfs.server, error status %d\n",WEXITSTATUS(res));
    return -1;
  }

  if ( chown("/usr/sbin/nexfs.server",0,0) == -1 )
  {
    printf("ERR: failed to chown to owner and group root /usr/sbin/nexfs.server\n");
    return -1;
  }

  if ( chmod("/usr/sbin/nexfs.server",0550) == -1 )
  {
    printf("ERR: failed to chmod to 550 /usr/sbin/nexfs.server\n");
    return -1;
  }

  snprintf(downloadcmd,2048,"%s %s 2>&1 | grep -q 'HTTP/2 200'",curlgetnexfscli,nexfscliurl);
  res=system(downloadcmd);

  if (( res == -1 ) || ( res == 256 ))
  {
    snprintf(downloadcmd,2048,"%s %s",wgetnexfscli,nexfscliurl);
    res=system(downloadcmd);

    if (( res == -1 ) || ( res == 256 ))
    {
      printf("ERR: failed to download latest release of nexfscli\n");
      return -1;
    }
  }

  if ( WEXITSTATUS(res) != 0 )
  {
    printf("ERR: failed to download latest release of nexfscli\n");
    return -1;
  }

  if ( chown("/usr/bin/nexfscli",0,0) == -1 )
  {
    printf("ERR: failed to chown to owner and group root /usr/bin/nexfscli\n");
    return -1;
  }

  if ( chmod("/usr/bin/nexfscli",0550) == -1 )
  {
    printf("ERR: failed to chmod to 550 /usr/bin/nexfscli\n");
    return -1;
  } 
  return 0;
}

int checkinstalledbinaries()
{
  int res=0;
  struct stat stbuf;

  if ( stat("/usr/sbin/nexfs.server",&stbuf) != 0 )
  {
    printf("ERR: Cannot stat /usr/sbin/nexfs.server, error %s\n",strerror(errno));
    return -1;
  }

  res=system("/usr/sbin/nexfs.server --version");

  if ( res == -1 )
  {
    printf("ERR: Cannot run /usr/sbin/nexfs.server, error %s\n",strerror(errno));
    return -1;
  }

  if ( res != 0 )
  {
    printf("ERR: '/usr/sbin/nexfs.server --version' failed to return version number\n");
    return -1;
  }

  if ( stat("/usr/bin/nexfscli",&stbuf) != 0 )
  {
    printf("ERR: Cannot stat /usr/bin/nexfscli, error %s\n",strerror(errno));
    return -1;
  }

  res=system("/usr/bin/nexfscli release get nexfscli");

  if ( res == -1 )
  {
    printf("ERR: Cannot run /usr/bin/nexfscli, error %s\n",strerror(errno));
    return -1;
  }

  if ( res != 0 )
  {
    printf("ERR: '/usr/bin/nexfscli release get nexfsci' failed to return version number\n");
    return -1;
  }

  return 0;
}

int installsystemdservice()
{
  int fd=-1;
  int res=0;
  struct stat stbuf;
  const char * servicefile = "/etc/systemd/system/nexfs.service";

  const char * nexfsservice =  
    "[Unit]\n"
    "Description=Nexustorage Nexfs Storage Server\n"
    "Wants=network-online.target\n"
    "After=network-online.target\n"
    "DefaultDependencies=no\n"
    "Conflicts=shutdown.target\n"
    "Before=shutdown.target\n"
    "\n"
    "[Service]\n"
    "Type=forking\n"
    "PIDFile=/run/nexfs.pid\n"
    "ExecStart=/usr/bin/nexfscli server start\n"
    "ExecStop=/usr/bin/nexfscli server stop\n"
    "\n"
    "[Install]\n"
    "WantedBy=sysinit.target\n" ;

  if ( stat("/usr/bin/systemctl",&stbuf) != 0 )
  {
    if ( errno == ENOENT )
      printf("Cannot install service on non (or non standard) systemd system\n");
    else
      printf("Cannot install serivce in systemd, errno %s\n", strerror(errno));

    return -1;
  }
  
  fd=open(servicefile,O_WRONLY | O_TRUNC | O_CREAT, 0644 );

  if ( fd == -1 ) 
  {
    printf("Failed to open or create service file %s, errno %s\n", servicefile, strerror(errno));
    return -1;
  }

  if ( pwrite(fd,nexfsservice,strlen(nexfsservice),0) == -1 )
  {
    printf("Failed to write to contents of service file %s, errno %s\n", servicefile, strerror(errno));
    close(fd);
    return -1;
  }
  close(fd);

  if ( (res = system("/usr/bin/systemctl daemon-reload")) != 0 )
  {
    printf("Failed to write to reload systemctl, returned %d\n", res);
    return -1;
  }

  if ( (res = system("/usr/bin/systemctl enable nexfs")) == -1 )
  {
    printf("Failed to enable nexfs in systemctl, returned %d\n", res);
    return -1;
  }

  printf("\nNexfs Installation completed\n"); 
  return 0;
}

int getservercounters()
{
  char buf[65536] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  int readoffset=0;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,2224,"%s/%s/getcounters",mountpoint,GFSCMDTAG);

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

  while ( (res = pread(lcp,buf,65536,readoffset)) > 0 )
  {
    if ( res < 65536 )
      buf[res]=0;
    
    readoffset+=(res-1);

    printf("%s",buf);
  }

  if ( res < 0 )
  {
     printf("ERR: failed to read stats from nexfs errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }


  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request server stats errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;
}


int getserverstats(int quite)
{
  char buf[65536] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  int readoffset=0;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  while ( (res = pread(lcp,buf,65536,readoffset)) > 0 )
  {
    if ( res < 65536 )
      buf[res]=0;
    
    readoffset+=(res-1);

    if ( quite == 0 ) printf("%s",buf);
  }

  if ( res < 0 )
  {
     printf("ERR: failed to read stats from nexfs errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }


  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request server stats errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  return 0;
}

int getliveconfig(char *CONFTAG, char *valuebuf, int bufsize)
{
  int res=0;
  int lcp;
  char returnbuf[4096];
  char TAGPATH[4500];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",returnbuf,sizeof(returnbuf),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,4500,"%s/%s/%s",returnbuf,GFSCONFIGTAG,CONFTAG);

  lcp = open( TAGPATH, O_RDONLY); 

  if (lcp == -1) 
  {
    if ( getserverstats(1) == 0 )
     printf("Failed to read requested value (is the requested configuration valid?)");

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

  if ( check_args(argc, 4, GREATER )) return -1;

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

int showfiletierusage(char *filename,int option )
{
  char buf[65537] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",returnbuf,sizeof(returnbuf),0);

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

  res=gfs_validateconfvalue(CONFTAG,valuebuf, 1);

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

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",returnbuf,sizeof(returnbuf),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(TAGPATH,2224,"%s/%s/%s",returnbuf,GFSCONFIGTAG,CONFTAG);

  lcp = open( TAGPATH, O_WRONLY|O_CREAT|O_TRUNC,0600); 

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

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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
  else if ( check_args(argc, 4, GREATER )) return -1;
  else if ( strcmp(argv[3],"jobid") == 0 )
  {
    if ( check_args(argc, 5, GREATER )) return -1;

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
      if ( (res=gfs_getconfig(GFSVALUE,GFSCONFIGTAGS[loop],returnbuf,sizeof(returnbuf),0)) != 0 ) return res;
    }
    else
    {
      if (( strcmp(GFSCONFIGTAGS[loop],"NEXFSCMD") != 0 )  && ( strcmp(GFSCONFIGTAGS[loop],"ROOTONLYACCESS") != 0 )) // NEXFSCMD is not a liveconfig
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

  if ( check_args(argc, 3, GREATER )) return -1;

  if ( strcmp(argv[2],"dumpall") == 0 )
  {
    return dumpconfig(1);
  }

  if ( check_args(argc, 4, GREATER )) return -1;

  if ( strcmp(argv[2],"get") == 0 )
  {
    if ( strcmp(argv[3],"value") == 0 )
    {
      if ( check_args(argc, 5, GREATER )) return -1;

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
      if ( check_args(argc, 5, EQUAL )) return -1;

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

    if ( check_args(argc, 5, GREATER )) return -1;

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
  char searchstr[4097];
  char searchdebugstr[4096];
  char mountpoint[2048];
  char *ptr;
  int res=1;

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"NEXFSCMD",NEXFSCMD,2048,0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs NEXFSCMD from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  snprintf(searchstr,4097,"%s %s ",NEXFSCMD,mountpoint);
  snprintf(searchdebugstr,4096,"nexfs.server-debug %s ",mountpoint);

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

      ptr=strstr(buf,searchdebugstr);

      if ( ptr != NULL )
      {
        return 1;
      }
    }
  }

  return 0;
}

int getlicensedetails()
{
  char buf[65536] = { 0 };
  char mountpoint[2048];
  int res;
  int lcp;
  char TAGPATH[2224];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  snprintf(TAGPATH,2224,"%s/%s/licensedetails",mountpoint,GFSCMDTAG);

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
     printf("ERR: failed to send empty buffer to initate server license details with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = pread(lcp,buf,65536,0);

  if ( res < 0 )
  {
     printf("ERR: failed to read license details from server with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }

  printf("%s",buf);

  res = close(lcp);

  if (res < 0 )
  {
     printf("ERR: failed to request server license details with errno %d - %s\n",errno,strerror(errno));
     return -errno;
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

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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
     printf("ERR: failed to send empty buffer to initate server status with errno %d - %s\n",errno,strerror(errno));
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
    res=gfs_getconfig(GFSVALUE,ENABLEDVAR,enabled,sizeof(enabled),0);

    if ( res != 0 )
    {
      printf("ERROR: Cannot read config value %s\n",ENABLEDVAR);
      return -1;
    }

    if ( atoi(enabled) == 0 )
    {
      return 0;
    }
  }

  res=gfs_getconfig(GFSVALUE,CONFVAR,thedir,sizeof(thedir),0);

  if ( res != 0 )
  {
    printf("ERROR: Cannot read config value %s\n",CONFVAR);
    return -1;
  }

  res=stat(thedir,&stbuf);

  if ( res != 0 )
  {
    printf("WARNING: cannot stat directory %s configured for %s \n",thedir, CONFVAR);
  }
  else if ( (stbuf.st_mode & S_IFMT) != S_IFDIR )
  {
    printf("ERROR: value %s for configuration %s is not a directory \n",thedir, CONFVAR);
    return -1;
  }

  return 0;
}

int startserver(int DEBUG)
{
  int res=0;
  char mountpoint[2048];
  char ALLOWOTHERS[2];
  char NEXFSCMD[2048];
  char CMD[4400];
  char LOCALCMD[4402];

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs MOUNTPOINT from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  if ( DEBUG == 1 )
  {
    strcpy(NEXFSCMD,"nexfs.server-debug");
    res=0;
  }
  else
    res=gfs_getconfig(GFSVALUE,"NEXFSCMD",NEXFSCMD,sizeof(NEXFSCMD),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs NEXFSCMD from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"ROOTONLYACCESS",ALLOWOTHERS,sizeof(ALLOWOTHERS),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs ROOTONLYACCESS from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  if ( ismounted() == 1 )
  {
    printf("nexfs is registrered by the kernel as mounted for mountpoint %s, try 'umount -f %s' before starting nexfs \n",mountpoint,mountpoint);
    return -1;
  }

  checkdirectoryexists("T1SDIR",NULL);
  checkdirectoryexists("T2SDIR","T2SREPLICATIONMODE"); 
  checkdirectoryexists("T1DDIR","T1DDIRENABLED"); 
  checkdirectoryexists("T2DDIR","T2DDIRENABLED"); 

  if (  atoi(ALLOWOTHERS) == 0 ) 
  {
    snprintf(CMD,4400,"%s --confdir=%s -o allow_other,noforget %s",NEXFSCMD,GFSCONFDIR,mountpoint);
  }
  else
  {
    snprintf(CMD,4400,"%s  --confdir=%s -o noforget %s",NEXFSCMD,GFSCONFDIR,mountpoint);
  }

  snprintf(LOCALCMD,4402,"./%s",CMD);

  res=system(CMD);

  if ( res != 0 )
  {
    printf("\nnexfs.server not started, check any error messages reported and that nexfs.server is in the search path, attempt returned %d\n",res);

    if ( res == 33536 )
      return -1;

    res=system(LOCALCMD);

    if ( res != 0 ) 
    {
      printf("\nFailed to start nexfs.server, check configurations NEXFSCMD and MOUNTPOINT are set correct, currently set to %s and %s, error code given %d\n",NEXFSCMD,mountpoint,res);
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
  int fd=0;
  char mountpoint[2048] = { 0 };
  char currentdir[2048] = { 0 };
  char buf[65536] = { 0 };
  char TAGPATH[2560] = { 0 };

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  if ( getcwd(currentdir,2048) == NULL )
  {
    printf("Failed to get current working directory: %s\n",strerror(errno));
    return -1;
  }

  if ( strncmp(currentdir,mountpoint,strlen(mountpoint)) == 0 )
  {
    if ( strlen(currentdir) == strlen(mountpoint) ) 
    {
      printf("Cannot stop nexfs server from within its own mountpoint\n");
      return -1;
    }
    
    if ( strlen(currentdir) > strlen(mountpoint) )
      if ( currentdir[strlen(mountpoint)] == '/' )
      {
        printf("Cannot stop nexfs server from within its own mountpoint\n");
        return -1;
      }
  }

  if ( force == 1 )
  {
    res = umount2(mountpoint,MNT_DETACH);
    sleep(1);
    res = umount2(mountpoint,MNT_FORCE);
    printf("Sent hard shutdown request to Nexfs\n");
    return 0;
  } 

  snprintf(TAGPATH,2224,"%s/%s/servershutdown",mountpoint,GFSCMDTAG);

  fd = open( TAGPATH, O_RDWR|O_DIRECT ); 

  if (fd == -1) 
  {
    print_connecterr();
    return -errno;
  }

  strncpy(buf,"CONFIRMSHUTDOWN",65536);

  res = pwrite(fd, buf,65536 ,0 );
  if (res < 0 )
  {
     printf("ERR: failed to send shutdown confirmation with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  } 

  res = pread(fd,buf,65536,0);

  if ( res < 0 )
  {
     printf("ERR: failed to confirm shutdown with errno %d - %s\n",errno,strerror(errno));
     return -errno;
  }

  res = close(fd);

  printf("Issued shutdown request to Nexfs\n");
    
  return 0;
}

int openstructurefile(char *filename, uint64_t *sfp)
{
  int res;
  char returnbuf[2048];
  char SDIRFILE[8192];

  res=gfs_getconfig(GFSVALUE,"T1SDIR",returnbuf,sizeof(returnbuf),0);

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
  struct stat stbuf = {0}; 

  if ( ( res=openstructurefile(filename,&sfp) ) < 0 ) return -errno;

  if ( option == 1 ) 
  {
    if ( (res = fgetxattr(sfp, "user.nexfsgfsid", returnbuf, 37)) == -1 )
    {
      printf("ERROR: Failed to retrive UID, error - %s\n",strerror(errno));
      return -errno;
    }
    returnbuf[res]=0;
    printf("SID: %s\n", returnbuf);

   if ( (res = fgetxattr(sfp, "user.nexfsgfscid", returnbuf, 37)) == -1 )
    {
      printf("ERROR: Failed to retrive UID, error - %s\n",strerror(errno));
      return -errno;
    }
    returnbuf[res]=0;
    printf("SCID: %s\n", returnbuf);

  }

  if ( fstat(sfp,&stbuf) != 0 )
  {
    printf("ERROR: Failed to stat requested file, error - %s\n",strerror(errno));
    return -errno;
  }

  printf("File Size: %ld\n",stbuf.st_size);
  
  if ( (res = fgetxattr(sfp, "user.nexfsdatapartsize", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive datapartsize, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Datapartsize: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfsallocatedchunks", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive allocatedchunks, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Allocated Data Chunks: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfsmintiered", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive mintiered, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Mintiered: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfsmaxtiered", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive maxtiered, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Maxtiered: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfslockedtotier", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive lockedtotier, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("LockToTier: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfssmartprotected", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive smartprotected, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("SmartProtected: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfsoldesttier1datafile", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive oldesttier1datafile, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Oldesttier1datafile: %s\n", returnbuf);

  if ( (res = fgetxattr(sfp, "user.nexfsoldesttier2datafile", returnbuf, 33)) == -1 )
  {
    printf("ERROR: Failed to retrive oldesttier2datafile, error - %s\n",strerror(errno));
    return -errno;
  }
  returnbuf[res]=0;
  printf("Oldesttier2datafile: %s\n", returnbuf);


  if ( (res = fgetxattr(sfp, "user.nexfsstructureversion", returnbuf, 33)) == -1 )
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
  if ( check_args(argc, 4, GREATER )) return -1;

  if ( strcmp(argv[2],"info") == 0 )
  {
    if ( argc == 5 )
    {
      if ( strcmp(argv[4],"-sid") == 0)
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

  char *DEFAULTS[][4][32] = { 
    { {"T3USEHTTPS"}, {"0"}, {"1" }, {"1"}},
    { {"T3S3USEVIRTUALHOST"}, {"0"}, {"1"}, {"1"}},
    { {"T3S3SIGNATUREV4"}, {"1"}, {"1"}, {"1"}},
    { {"T3S3ENABLEREGION"}, {"0"}, {"1"}, {"0"}},
    { {"T3S3COMPATIBLE"}, {"1"}, {"0"}, {"0"}},
    { {"T3S3URL"}, {"localhost"}, {"amazonaws.com"}, {"filebase.com"}},
    { {"T3S3PORT"}, {"9000"}, {"443"},{"443"}},
    { {"T3S3RETRYSLEEP"}, {"1"}, {"1"}, {"1"}},
    { {"T3S3RETRIES"}, {"3"}, {"3"}, {"3"}},
    { {"T3S3RETRY404"}, {"0"}, {"0"}, {"0"}}
  };

  char MSG[3][400] = {
    "\nDefaults loaded, please run and set the following\nnexfscli configfile set T3S3URL {minio host}\nnexfscli configfile set T3S3PORT {minio port}\nnexfscli configfile set T3S3BUCKET {bucket}\nnexfscli configfile set T3AWSAccessKeyId {Access Key}\nnexfscli configfile set T3AWSSecretAccessKey {secret access key}\n",
    "\nDefaults loaded, please run and set the following\nnexfscli configile set T3S3REGION {AWS Region}\nnexfscli configfile set T3S3BUCKET {S3 bucket}\nnexfscli configfile set T3AWSAccessKeyId {S3 Access Key}\nnexfscli configfile set T3AWSSecretAccessKey {S3 secret access key}\n",
    "\nDefaults loaded, please run and set the following\nnexfscli configfile set T3S3BUCKET {S3 bucket}\nnexfscli configfile set T3AWSAccessKeyId {S3 Access Key}\nnexfscli configfile set T3AWSSecretAccessKey {S3 secret access key}\n"
  };

  printf("\nPlease confirm overwrite of config file parms with requested defaults (y/n): ");

  while ( fgets(confirm,3,stdin) == NULL );


  if ( (strncmp(confirm,"y",1) == 0 ) || ( strncmp(confirm,"Y",1) == 0 ) || ( strncmp(confirm,"yes",3) == 0) || ( strncmp(confirm,"YES",3) == 0 ))
  {
    for ( loop=0; loop < defrows; loop++ )
    {
      if ( option == 0 ) // 0=minio, 1=AWS S3, 2=FileBase
        strcpy(newvalue,*DEFAULTS[loop][1]);
      else if ( option == 1 )
        strcpy(newvalue,*DEFAULTS[loop][2]);
      else 
        strcpy(newvalue,*DEFAULTS[loop][3]);
    
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
    else if ( option == 1 )
      printf("%s",MSG[1]);
    else
      printf("%s",MSG[2]);

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

  if ( check_args(argc, 3, GREATER )) return -1;

  if ( strcmp(argv[2],"dumpall") == 0 )
  {
    return dumpconfig(0);
  }

  if ( check_args(argc, 4, GREATER )) return -1;

  if ( argc != 5 ) 
  {
    if ( strcmp(argv[3],"loglevel") != 0 )
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
      else if ( strcmp(argv[4],"filebase") == 0  )
        return configfiles_setdefaults(2);

      printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
      return -1;
    }

    if ( strcmp(argv[3],"loglevel") == 0 )
      argv[3]=LOGLEVELTAG;

    if ( argc < 5 )
      res=-1;
    else
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
      res=gfs_getconfig(GFSTAG,argv[4],returnbuf,sizeof(returnbuf),0);

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
      res=gfs_getconfig(GFSVALUE,"GFSLOGLEVEL",returnbuf,sizeof(returnbuf),0);
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
      res=gfs_getconfig(GFSVALUE,argv[4],returnbuf,sizeof(returnbuf),0);

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
      res=gfs_getconfig(GFSHELP,argv[4],returnbuf,sizeof(returnbuf),0);

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
      res=gfs_getconfig(GFSTAG,argv[4],returnbuf,sizeof(returnbuf),0);

      if ( res > -1 )
      {
        printf("Tag Label = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVALUE,argv[4],returnbuf,sizeof(returnbuf),0);
        printf("Value = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSHELP,argv[4],returnbuf,sizeof(returnbuf),0);
        printf("Help Text = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVSTRING,argv[4],returnbuf,sizeof(returnbuf),0);
        printf("Validation:String = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVMIN,argv[4],returnbuf,sizeof(returnbuf),0);
        printf("Validation:Min(Value/StringLength) = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSVMAX,argv[4],returnbuf,sizeof(returnbuf),0);
        printf("Validation:Max(Value/StringLength) = '%s'\n",returnbuf);

        res=gfs_getconfig(GFSRESTART,argv[4],returnbuf,sizeof(returnbuf),0);
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

int accepttermsandconditions()
{
  char * args[] = { "-", "-", "set", "TERMSANDCONDITIONSACCEPTED", "1" };
  char accept[100] = { 0 };
  char ACCEPT[100] = { 0 };
  const int argc=5;
  int loop=0;

  if ( cmdlineswitches.accepttermsandcondition == 0 )
  {
    if ( cmdlineswitches.noprompt ==  1 )
      return 0;

    printf("Please review and accept the Nexustorage Terms and Conditions for using Nexfs\n");
    printf("A latest T&Cs are available at https://www.nexustorage.com/nexustorage-terms-and-conditions\n\n");

    while ( cmdlineswitches.accepttermsandcondition == 0 )
    {
      printf("\nDo you accept the Nexustorage Nexfs T&Cs (Yes/No)?");
      if ( fgets(accept,100,stdin) != NULL )
      {
        for ( loop=0; loop < strlen(accept)-1; loop++ )
        {
          ACCEPT[loop]=toupper(accept[loop]);
        }
        ACCEPT[loop+1]=0;

        if (strcmp(ACCEPT,"YES") == 0 )
         cmdlineswitches.accepttermsandcondition=1;
        else if ( strcmp(ACCEPT,"NO") == 0 )
          return 0;
      }
    }
  }

  
  return configfiles(argc,args);
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
  }
  else if ( strcmp(argv[4],"config") == 0 ) 
  {
    strcpy(CONFIGVAR,"SECURITYCONFGROUP");
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
      res=gfs_getconfig(GFSVALUE,CONFIGVAR,buffer,sizeof(buffer),0);
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
      res=gfs_getconfig(GFSVALUE,CONFIGVAR,buffer,sizeof(buffer),0);
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
  if ( check_args(argc, 6, GREATER )) return -1;

  if ( argc == 6 )
  {
    if ( strcmp(argv[3],"get") != 0 )
    {
      printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
      return -1;
    }
  }
  else if ( argc == 7 )
  {
    if ( strcmp(argv[3],"set") != 0 )
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

int iscsi_show(int argc, char *argv[])
{
  int res=0;
  int filein=-1;
  int bytesread=0;
  char parms[128] = { 0 };
  char inbuf[65536*4] = { 0 };
  char mountpoint[2048] = { 0 };
  char NEXFSOPURL[2224] = { 0 };

  if ( check_args(argc, 4, GREATER )) return -1;

  if (( strcmp(argv[2],"show") == 0 ) && (( strcmp(argv[3],"targets") == 0 ) || ( strcmp(argv[3],"luns") == 0)))
  {
    strcpy(parms,"ISCSISHOWTARGETS");
  }
  else if (( strcmp(argv[2],"show") == 0 ) && ( strcmp(argv[3],"target") == 0 ))
  {
    if ( check_args(argc, 5, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISHOWTARGET=%s",argv[4]);
  }
  else if ( strcmp(argv[3],"accounts") == 0 ) 
  {
    strcpy(parms,"ISCSISHOWACCOUNTS");
  }
  else if ( strcmp(argv[3],"interfaces") == 0 ) 
  {
    strcpy(parms,"ISCSISHOWINTERFACES");
  }
  else if (( strcmp(argv[2],"show") == 0 ) && ( strcmp(argv[3],"sessions") == 0 )) 
  {
    strcpy(parms,"ISCSISHOWSESSIONS");
  }
  else if ( strcmp(argv[3],"targetsessions") == 0 )  
  {
    if ( check_args(argc, 5, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISHOWTARGETSESSIONS=%s",argv[4]);
  }
  else if ( strcmp(argv[3],"connections") == 0 ) 
  {
    strcpy(parms,"ISCSISTATCONNECTIONS");
  }
  else if ( strcmp(argv[3],"connection") == 0 )  
  {
    if ( check_args(argc, 5, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISTATCONNECTION=%s",argv[4]);
  }
  else if (( strcmp(argv[2],"stat") == 0 ) && ( strcmp(argv[3],"sessions") == 0 ))
  {
    strcpy(parms,"ISCSISTATSESSIONS");
  }
  else if ( strcmp(argv[3],"session") == 0 )  
  {
    if ( check_args(argc, 5, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISTATSESSION=%s",argv[4]);
  }
  else if (( strcmp(argv[2],"stat") == 0 ) && ( strcmp(argv[3],"targets") == 0 ))
  {
    strcpy(parms,"ISCSISTATTARGETS");
  }
  else if (( strcmp(argv[2],"stat") == 0 ) && ( strcmp(argv[3],"target") == 0 ))
  {
    if ( check_args(argc, 5, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISTATTARGET=%s",argv[4]);
  }
  else if ( strcmp(argv[3],"luns") == 0 )  
  {
    strcpy(parms,"ISCSISTATLUNS");
  }
  else if (( strcmp(argv[2],"stat") == 0 ) && ( strcmp(argv[3],"targetluns") == 0 ))
  {
    if ( check_args(argc, 5, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISTATTARGETLUNS=%s",argv[4]);
  }
  else if (( strcmp(argv[2],"stat") == 0 ) && ( strcmp(argv[3],"lun") == 0 ))
  {
    if ( check_args(argc, 6, EQUAL )) return -1;
    snprintf(parms,sizeof(parms),"ISCSISTATLUN=%s,%s",argv[4],argv[5]);
  }
  else
    return -1;

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638869/%s",mountpoint,parms);

  filein=open(NEXFSOPURL,O_RDONLY | O_DIRECT);
  if ( filein == -1 )
  {
    printf("%s: failed to open connection to Nexfs for reading, error %s",argv[0],strerror(errno));
    return -1;
  }

  while ( (res = pread(filein,inbuf+bytesread,65536,bytesread)) > 0 ) 
    {
      bytesread+=res;
      if ( bytesread == sizeof(inbuf) )
        break;
    }


  if (( bytesread == -1) || ( res == -1 )) 
  {
    printf("%s: failed to read data, error %s",argv[0],strerror(errno));
    if ( filein != -1 ) close(filein);
    return -1;
  }

  printf("%s\n",inbuf);

  return 0;
}

int iscsi (int argc, char *argv[])
{
  int filein=-1;
  int fileout=-1;
  const int GET=1;
  const int PUT=2;
  char inbuf[65536*4] = { 0 };
  char mountpoint[2048] = { 0 };
  char NEXFSOPURL[2100] = { 0 };
  int opp=-1;
  int bytesread=0;
  int byteswritten=0;
  int res=0;

  if ( check_args(argc, 2, GREATER )) return -1;

  if (( strcmp(argv[2],"show") == 0 ) || (strcmp(argv[2],"stat") == 0 )) 
    return iscsi_show(argc, argv);

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  if ( strcmp(argv[2],"getconf") == 0 )
  {
    opp=GET;
    if ( argc == 4 )
    {
      fileout = open(argv[3],O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU );
      if ( fileout == -1 )
      {
        printf("%s: failed to open %s for writting, error %s",argv[0],argv[3],strerror(errno));
        return -1;
      }
    }
     
    snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638869/ISCSIGETCONF",mountpoint);

    filein=open(NEXFSOPURL,O_RDONLY | O_DIRECT);
    if ( filein == -1 )
    {
      printf("%s: failed to open connection to Nexfs for reading, error %s",argv[0],strerror(errno));
      return -1;
    }

  }
  else if ( strcmp(argv[2],"putconf") == 0 )
  {
    opp=PUT;
    if ( argc == 4 )
    {
      filein = open(argv[3],O_RDONLY );
      if ( filein == -1 )
      {
        printf("%s: failed to open %s for reading, error %s",argv[0],argv[3],strerror(errno));
        return -1;
      }
    }
  }
  else
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  res=0;
  if ( filein == -1 )
     bytesread=fread(inbuf,sizeof( char ),sizeof(inbuf),stdin); 
  else
    while ( (res = pread(filein,inbuf+bytesread,65536,bytesread)) > 0 ) 
    {
      bytesread+=res;
      if ( bytesread == sizeof(inbuf) )
        break;
    }

  if (( bytesread == -1) || ( res == -1 )) 
  {
    printf("%s: failed to read data, error %s",argv[0],strerror(errno));
    if ( opp == GET) close(fileout); // this is the last thing we print so does not matter if we close stdout
    if ( filein != -1 ) close(filein);
    return -1;
  }

  bytesread=strlen(inbuf);

  if ( opp == GET )
  {
    if ( fileout == -1 )
     printf("%s\n",inbuf);
    else
    {
      byteswritten=0;
      do
      {
        res+=pwrite(fileout,inbuf,bytesread-byteswritten > 65536 ? 65536 : bytesread-byteswritten,byteswritten); 
        if ( res == -1 )
        {
          printf("%s: failed to write data, error %s",argv[0],strerror(errno));
          return -1;
        }
        byteswritten+=res; 
      } while ( byteswritten < bytesread);
    }
    if ( fileout != -1 ) close(fileout); // this is the last thing we print so does not matter if we close stdout
    if ( filein != -1 ) close(filein);
    return 0;
  }

  snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638869/ISCSIPUTCONF",mountpoint);

  fileout=open(NEXFSOPURL,O_WRONLY | O_DIRECT);
  if ( fileout == -1 )
  {
    printf("%s: failed to open connection to Nexfs for reading, error %s",argv[0],strerror(errno));
    if ( filein != -1 ) close(filein);
    return -1;
  }

  byteswritten=0;
  do
  {
    res+=pwrite(fileout,inbuf, 65536,byteswritten); 
    if ( res == -1 )
    {
      printf("%s: failed to write data, error %s",argv[0],strerror(errno));
      if ( filein != -1 ) close(filein);
      close(fileout);
      return -1;
    }
    byteswritten+=res; 
  } while ( byteswritten < bytesread);

  if ( filein != -1 ) close(filein);
  if ( fileout != -1 ) close(fileout);

  return 0; 
}

int nfs (int argc, char *argv[])
{
  int filein=-1;
  FILE *fileout=NULL;
  const int GET=1;
  const int PUT=2;
  char inbuf[262000] = { 0 };
  char mountpoint[2048] = { 0 };
  char NEXFSOPURL[2100] = { 0 };
  int opp=-1;
  int bytesread=0;
  int byteswritten=0;
  int res=0;
  int needforcewrite=0;

  if ( check_args(argc, 2, GREATER )) return -1;

  res=gfs_getconfig(GFSVALUE,"MOUNTPOINT",mountpoint,sizeof(mountpoint),0);

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

  if ( strcmp(argv[2],"getexports") == 0 )
  {
    opp=GET;
    if ( argc == 4 )
    {
      fileout = fopen(argv[3],"w");
      if ( fileout == NULL )
      {
        printf("%s: failed to open %s for writting, error %s",argv[0],argv[3],strerror(errno));
        return -1;
      }
    }
    else
      fileout=stdout;
     
    snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638869/GETNFSEXPORTS",mountpoint);

    unlink(NEXFSOPURL); // A simple hack to get around the need to otherwise do direct io, expected to fail, will remove any exports cached by the kernel  

    filein=open(NEXFSOPURL,O_RDONLY | O_DIRECT);
    if ( filein == -1 )
    {
      printf("%s: failed to open connection to Nexfs for reading, error %s",argv[0],strerror(errno));
      return -1;
    }
  }
  else if ( strcmp(argv[2],"putexports") == 0 )
  {
    opp=PUT;
    if ( argc == 4 )
    {
      filein = open(argv[3],O_RDONLY );
      if ( filein == -1 )
      {
        printf("%s: failed to open %s for reading, error %s",argv[0],argv[3],strerror(errno));
        return -1;
      }
    }
  }
  else
  {
    printf("%s: incorrect format, type %s help\n",argv[0],argv[0]);
    return -1;
  }

  res=0;
  if ( filein == -1 )
     bytesread=fread(inbuf,sizeof( char ),sizeof(inbuf),stdin); 
  else
    while ( (res = pread(filein,inbuf+bytesread,65536,bytesread)) > 0 ) 
    {
      bytesread+=res;
      if ( bytesread == sizeof(inbuf) )
        break;
    }

  if (( bytesread == -1) || ( res == -1 )) 
  {
    printf("%s: failed to read data, error %s",argv[0],strerror(errno));
    if ( opp == GET) fclose(fileout); // this is the last thing we print so does not matter if we close stdout
    if ( filein != -1 ) close(filein);
    return -1;
  }

  if ( opp == GET )
  {
    fprintf(fileout,"%s\n",inbuf);
    fclose(fileout); // this is the last thing we print so does not matter if we close stdout
    if ( filein != -1 ) close(filein);
    return 0;
  }

  // we will do some very basic checks of expected new exportfs json contents

  if ( bytesread == 0 ) 
    needforcewrite=1;
  else if ( strstr(inbuf,"\"nfsexports\":") == NULL )
    needforcewrite=1;
  else if ( strstr(inbuf,"\"exportdir\":") == NULL )
  {
    printf("%s: new exports invalid, not updated",argv[0]);
  }

  if (( needforcewrite == 1 ) && ( argc < 5 ))
  {
    printf("%s: force option required delete all configured exports",argv[0]);
    if ( filein != -1 ) close(filein);
    return -1;
  }
  else if (( needforcewrite == 1 ) && (( strcmp(argv[4],"-f") != 0 ) || ( strcmp(argv[4],"-force") != 0 )))
  {
    printf("%s: force option required to delete all configured exports",argv[0]);
    if ( filein != -1 ) close(filein);
    return -1;
  }

  snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638869/PUTNFSEXPORTS",mountpoint);

  fileout=fopen(NEXFSOPURL,"w");
  if ( fileout == NULL )
  {
    printf("%s: failed to open connection to Nexfs for reading, error %s",argv[0],strerror(errno));
    if ( filein != -1 ) close(filein);
    return -1;
  }

  byteswritten = fwrite(inbuf,sizeof(char),bytesread,fileout);

  if ( byteswritten == -1 )
  {
    printf("%s: failed to write new exports data, error %s",argv[0],strerror(errno));
    if ( filein != -1 ) close(filein);
    fclose(fileout);
    return -1;
  }

  if ( filein != -1 ) close(filein);
  fclose(fileout);

  // The following two unlinks are expected to fail, in doing so they will invalidate the kernel cache of these virtual files
  // nexfs tries had to get the kernal to invalidate these, but making the kernel fail a opp on the virtual files can help thgere removal from cache

  snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638869/GETNFSEXPORTS",mountpoint);
  unlink(NEXFSOPURL);
  snprintf(NEXFSOPURL,sizeof(NEXFSOPURL),"%s/2b339ae7a7f04e45960f8a3fcc638801",mountpoint);
  unlink(NEXFSOPURL);


  return 0; 
}

int server(int argc, char *argv[])
{
  if ( check_args(argc, 3, EQUAL )) return -1;

  if ( strcmp(argv[2],"start") == 0 )
  {
    return ( startserver(0) );
  }
  else if ( strcmp(argv[2],"startdebug") == 0 )
  {
    return ( startserver(1) );
  }
  else if ( strcmp(argv[2],"status") == 0 )
  {
    return ( getserverstatus() );
  }
  else if ( strcmp(argv[2],"license") == 0 )
  {
    return ( getlicensedetails() );
  }
  else if ( strcmp(argv[2],"stop") == 0 )
  {
    return ( stopserver(0) );
  }
  else if ( strcmp(argv[2],"forcestop") == 0 )
  {
    return ( stopserver(1) );
  }
  else if ( strcmp(argv[2],"counters") == 0 )
  {
    return ( getservercounters() );
  }
  else if ( strcmp(argv[2],"stats") == 0 )
  {
    return ( getserverstats(0) );
  }

  printf("%s: Unknown command format, type %s help\n",argv[0],argv[0]);

  return -1;
} 

int init()
{
  int res=0;
  struct stat stbuf = {0};
  errno=0;

  res = stat(GFSCONFDIR,&stbuf);

  if (( res != 0 ) && (errno != ENOENT ))
  {
    printf("%s: Failed to stat configuration directory %s, errno %d - %s\n",MYNAME,GFSCONFDIR,errno,strerror(errno));
    return -errno; 
  }

  if ( errno == ENOENT )
  {
    res = mkdir(GFSCONFDIR, 0755);

    if ( res != 0 )
    {
      printf("%s: Failed to create configuration directory %s, errno %d - %s\n",MYNAME,GFSCONFDIR,errno,strerror(errno));
      return -errno;
    }
  }

  res=generateconfigfiles();

  return res;
}

int copyxattrs(char *src, char *dst)
{
  int res=0;
  size_t xattrslen, keylen, vallen; 
  size_t valmemlen=0;
  char *buf, *key, *val;
  errno=0;

  xattrslen=listxattr(src,NULL,0);

  if ( xattrslen == -1 )
  {
    printf("%s copyxattrs failed to list xattrs for %s, error = %s",MYNAME,src,strerror(errno));
    return -errno;
  }

  if ( xattrslen == 0 )
    return 0;

  val=0;
  buf=malloc(xattrslen);

  if ( buf == NULL )
  {
    printf("%s copyxattrs failed to allocated %ld bytes of memory, error = %s",MYNAME,xattrslen,strerror(errno));
    return -errno;
  }

  xattrslen=listxattr(src,buf,xattrslen);

  if ( xattrslen == -1 )
  {
    printf("%s copyxattrs failed to get list of xattrs for %s, error = %s",MYNAME,src,strerror(errno));
    free(buf);
    return -errno;
  }

  key = buf;
  val=NULL;

  while ( xattrslen> 0 )
  {
    vallen=getxattr(src,key,NULL,0);
    if ( vallen < 0 )
    {
      printf("%s copyxattrs failed to get value length for xattr %s from %s, error = %s",MYNAME,key,src,strerror(errno));
      free(buf);
      if ( val != NULL ) free(val);
      return -errno;
    }

    if ( vallen > valmemlen )
    {
      val=realloc(val,vallen+1);

      if ( val == NULL )
      {
        printf("%s copyxattrs failed to allocated %ld bytes of memory for xattr val, error = %s",MYNAME,xattrslen,strerror(errno));
        free(buf);
        return -errno;
      }
      valmemlen=vallen;
    }
    memset(val,0,vallen+1);

    vallen=getxattr(src,key,val,vallen);

    if ( vallen < 0 )
    {
      printf("%s copyxattrs failed to get value for xattr %s from %s, error = %s",MYNAME,key,src,strerror(errno));
      if ( val != NULL ) free(val);
      free(buf);
      return -errno;
    }

    if ( vallen == 0 )
      res=setxattr(dst,key,"",0,0);
    else
    {
      res=setxattr(dst,key,val,vallen,0);
    }
    
    if ( res < 0 ) 
    {
      printf("%s copyxattrs failed to set xattr '%s' value '%s' length %ld on file %s, error = %s",MYNAME,key,val,vallen,dst,strerror(errno));
      if ( val != NULL ) free(val);
      free(buf);
      return -errno;
    }

    keylen = strlen(key) + 1;
    xattrslen -= keylen;
    key += keylen;
  }
  if ( val != NULL ) free(val);
  free(buf);
  return 0;
}  


int replicatedirent(char *src, char *dst)
{
  int res=0;
  struct stat srcstat;
  struct timespec *srctimespec[2];


  if ( stat(src, &srcstat) != 0 )
  { 
    res=errno;
    printf("%s replicatedirent, failed to stat %s error %s",MYNAME,src,strerror(errno));
    errno=res;
    return -errno;
  }
  

  if ( (srcstat.st_mode  & S_IFMT ) != S_IFDIR)
  {
    if ( copyxattrs(src, dst) != 0 ) 
    { 
      res=errno;
      printf("%s replicatedirent, copyxattrs failed from %s to %s, error %s",MYNAME,src,dst,strerror(errno));
      errno=res;
      return -errno;
    }

    if ( truncate(dst, srcstat.st_size) != 0 )
    { 
      res=errno;
      printf("%s: replicatesdirent, failed to truncate %s to size %ld, error %s",MYNAME,dst,srcstat.st_size,strerror(errno));
      errno=res;
      return -errno;
    }
  }

  if ( chown(dst, srcstat.st_uid, srcstat.st_gid) != 0 )
  { 
    res=errno;
    printf("%s: replicatesdirent, failed to chown %s, error %s",MYNAME,dst,strerror(errno));
    errno=res;
    return -errno;
  }

  if ( chmod(dst, srcstat.st_mode) != 0 )
  { 
    res=errno;
    printf("%s: replicatesdirent, failed to chmod %s, error %s",MYNAME,dst,strerror(errno));
    errno=res;
    return -errno;
  } 


  srctimespec[0]=&srcstat.st_atim;
  srctimespec[1]=&srcstat.st_mtim;

  if ( utimensat(-1,dst, *srctimespec,AT_SYMLINK_NOFOLLOW) != 0 )
  { 
    res=errno;
    printf("%s:replicatedirent, failed to futimens %s, error %s",MYNAME,dst,strerror(errno));
    errno=res;
    return -errno;
  } 

  return 0;
}


int syncstructure(char *T1SDIR, char *T2SDIR, char *sdir, int quite)
{
  DIR *t1sdir;
  struct dirent *dirp;
  int res=0;
  int dfp=0;
  int sfp=0;
  int bytesread=0;
  int createentry=0;
  char sourcefolder[8196]= { 0 };
  char sourcename[8196]= { 0 };
  char destinationname[8196]= { 0 };
  char nsdir[8196] = { 0 };
  char buf[8192];
  struct stat stbuf = {0};
  struct stat dstbuf = {0};

  snprintf(sourcefolder,8196,"%s/%s",T1SDIR, sdir);

  t1sdir = opendir(sourcefolder);
   
  if (t1sdir == NULL )
  {
    printf("%s: Failed to open source directory %s, error %d - %s",MYNAME,sourcefolder,errno,strerror(errno));
    return -errno;
  }

  while ((dirp = readdir(t1sdir)) != NULL)
  {
    createentry=0;
    errno=0;

    if ((strcmp(dirp->d_name, ".") != 0 ) && (strcmp(dirp->d_name, "..") != 0) && ( strcmp(dirp->d_name,".statustestfile-9cbe477e-b2a8-11eb-b189-dff8ac5067a1") != 0))
    {
      snprintf(sourcename,sizeof(sourcename),"%s/%s/%s",T1SDIR,sdir,dirp->d_name);
      snprintf(destinationname,sizeof(destinationname),"%s/%s/%s",T2SDIR,sdir,dirp->d_name);

      if ( (res = lstat(sourcename,&stbuf)) != 0 )
      {
        if ( errno != ENOENT)  printf("%s: Could not stat source %s, error returned %s",MYNAME,sourcename,strerror(errno)); 
        return -errno;
      }

      if ( (res = lstat(destinationname,&dstbuf)) != 0 )
      {
        if ( errno != ENOENT )
        {
          if ( errno != ENOENT)  printf("%s: Could not stat destination %s, error returned %s",MYNAME,destinationname,strerror(errno)); 
          return -errno;
        }
        createentry=1;
      }

      if ( errno == 0 )
      {
        if ( (stbuf.st_mode  & S_IFMT ) != ( dstbuf.st_mode & S_IFMT ))
        {
          if ( unlink(destinationname) != 0 )
          {
            if ( errno != ENOENT)  printf("%s: Could not remove existing invalid destination entry %s, error returned %s",MYNAME,destinationname,strerror(errno)); 
            return -errno;
          }
          else
            createentry=1;
        }
      }

      if ( createentry )
      {
        if ( (stbuf.st_mode  & S_IFMT ) == S_IFDIR)
        {
          if ( mkdir(destinationname,stbuf.st_mode) != 0 )
          {
            if ( errno != ENOENT)  
            {
              printf("%s: Could not create destination directory %s, error returned %s",MYNAME,destinationname,strerror(errno));
              return -errno;
            }
          }
        }
        else
        {
          dfp=-1;
          if ( (dfp = open(destinationname,O_WRONLY | O_CREAT, stbuf.st_mode)) == -1 )
          {
            printf("%s: Could not open destination entry %s, error returned %s",MYNAME,destinationname,strerror(errno)); 
            return -errno;
          }

          sfp=-1;
          if ( (sfp = open(sourcename,O_RDONLY )) == -1 )
          {
            printf("%s: Could not open source directory entry %s, error returned %s",MYNAME,destinationname,strerror(errno)); 
            return -errno;
          }

          if ( stbuf.st_size > 0 )
          {
            bytesread=0;
            while ( (bytesread=read(sfp,buf,sizeof(buf))) > 0 )
            {
              if (buf[0] == 0 )
                break;

              if ( write(dfp,buf,bytesread) != bytesread )
              {
                printf("%s: Could not write %d bytes to destination file %s, error if returned %s",MYNAME,bytesread,destinationname,strerror(errno)); 
                return -1;
              }
            }
          }
       
          if ( sfp != -1 ) close(sfp);
          if ( dfp != -1 ) close(dfp);
        }
      }

      if ( (stbuf.st_mode  & S_IFMT ) == S_IFDIR)
      {
        snprintf(nsdir,sizeof(nsdir),"%s/%s",sdir,dirp->d_name);
        if ( syncstructure(T1SDIR, T2SDIR, nsdir, quite) != 0 )
          return -errno;
      }

      if ( replicatedirent(sourcename, destinationname) != 0 ) 
        return -errno;
    }
  }
  return 0;
}

int syncstructurefilesystems(int argc, char *argv[])
{
  int res=0;
  int REPLICATIONMODE=0;
  char T1SDIR[8192]= {0};
  char T2SDIR[8192]= {0};
  char buffer[10] = { 0 };

  res=gfs_getconfig(GFSVALUE,"T1SDIR",T1SDIR,sizeof(T1SDIR),0);

  if ( res == -1 )
  {
    printf("%s: Failed to retrieve nexfs T1SDIR from configuration data, errno %d - %s\n",MYNAME,errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"T2SREPLICATIONMODE",buffer,sizeof(buffer),0);
  if ( res == -1 )
  {
    printf("%s: Failed to retrieve nexfs T2SREPLICATIONMODE from configuration data, errno %d - %s\n",MYNAME,errno,strerror(errno));
    return -errno;
  }

  REPLICATIONMODE=atoi(buffer);

  if ( REPLICATIONMODE > 0 )
  {
    res=gfs_getconfig(GFSVALUE,"T2SDIR",T2SDIR,sizeof(T2SDIR),0);
    if ( res == -1 )
    {
      printf("%s: Failed to retrieve nexfs T2SDIR from configuration data, errno %d - %s\n",MYNAME,errno,strerror(errno));
      return -errno;
    }

    printf("%s: Starting structure replication, this may take a long time and will need to be restarted if interrupted\n",argv[0]);

    if ( syncstructure(T1SDIR, T2SDIR, "", 0) != 0 )
      return -errno;
  }
  else
  {
    printf("%s: Structure data replication not configured\n",argv[0]);
  }

  printf("%s: Structure data replication completed successfully\n",argv[0]);

  return 0;
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
  int REPLICATIONMODE;
  char T1DDIR[8192]= {0};
  char T2DDIR[8192]= {0};
  char T2SDIR[8192]= {0};
  char basefoldername[8196]= { 0 };
  char foldername[8204]= { 0 };
  char buffer[10] = { 0 };

  res=gfs_getconfig(GFSVALUE,"T1SDIR",buffer,sizeof(buffer),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T1SDIR from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }
  
  snprintf(basefoldername,8196,"%s/1e",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }

  snprintf(basefoldername,8196,"%s/de",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }

  snprintf(basefoldername,8196,"%s/df",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }

  snprintf(basefoldername,8196,"%s/cf",buffer);
  res = mkdir(basefoldername, 0755);

  if (( res != 0 ) && ( errno != EEXIST ))
  {
    printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
    return -errno;
  }

  res=gfs_getconfig(GFSVALUE,"T2SREPLICATIONMODE",buffer,sizeof(buffer),0);
  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T2SREPLICATIONMODE from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  REPLICATIONMODE=atoi(buffer);

  if ( REPLICATIONMODE > 0 )
  {
    res=gfs_getconfig(GFSVALUE,"T2SDIR",T2SDIR,sizeof(T2SDIR),0);
    if ( res == -1 )
    {
      printf("Failed to retrieve nexfs T2SDIR from configuration data, errno %d - %s\n",errno,strerror(errno));
      return -errno;
    }

    snprintf(basefoldername,8196,"%s/1e",T2SDIR);
    res = mkdir(basefoldername, 0755);

    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
      return -errno;
    }

    snprintf(basefoldername,8196,"%s/de",T2SDIR);
    res = mkdir(basefoldername, 0755);

    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
      return -errno;
    }

    snprintf(basefoldername,8196,"%s/df",T2SDIR);
    res = mkdir(basefoldername, 0755);

    if (( res != 0 ) && ( errno != EEXIST ))
    {
      printf("%s: Failed to create structure directory %s, errno %d - %s\n",argv[0],basefoldername,errno,strerror(errno));
      return -errno;
    }

  }

  res=gfs_getconfig(GFSVALUE,"T1DDIRENABLED",buffer,sizeof(buffer),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T1DDIRENABLED from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  T1DDIRENABLED=atoi(buffer);

  res=gfs_getconfig(GFSVALUE,"T2DDIRENABLED",buffer,sizeof(buffer),0);

  if ( res == -1 )
  {
    printf("Failed to retrieve nexfs T2DDIRENABLED from configuration data, errno %d - %s\n",errno,strerror(errno));
    return -errno;
  }

  T2DDIRENABLED=atoi(buffer);

  if ( T1DDIRENABLED == 1 )
  {
    res=gfs_getconfig(GFSVALUE,"T1DDIR",T1DDIR,sizeof(T1DDIR),0);

    if ( res == -1 )
    {
      printf("Failed to retrieve nexfs T1DDIR from configuration data, errno %d - %s\n",errno,strerror(errno));
      return -errno;
    }
  }

  if ( T2DDIRENABLED == 1 )
  {
    res=gfs_getconfig(GFSVALUE,"T2DDIR",T2DDIR,sizeof(T2DDIR),0);

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
        snprintf(basefoldername,8196,"%s/%02x",T1DDIR,loop);
      else if (( loop1 == 1 ) && ( T2DDIRENABLED == 1 ))
        snprintf(basefoldername,8196,"%s/%02x",T2DDIR,loop);

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
        snprintf(foldername,8204,"%s/%02x",basefoldername,loop2);

        res = mkdir(foldername, 0755);

        if (( res != 0 ) && ( errno != EEXIST ))
        {
          printf("%s: Failed to create data directory %s, errno %d - %s\n",argv[0],foldername,errno,strerror(errno));
          return -errno;
        }

        for (loop3=0; loop3<16; loop3++)
        {
          snprintf(foldername,8204,"%s/%02x/%0X",basefoldername,loop2,loop3);

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

int installnexfs()
{
  int thepkgmgr=0;
  int res=0;

  if ( cmdlineswitches.noinstallpackages == 0 )
  {
    thepkgmgr=whichpackagemanager();

    if ( thepkgmgr == apt )
      res=installaptpackagedepnds();
    else if ( thepkgmgr == rpm ) 
      res=installrpmpackagedepends();
    else
      res=0;

    if ( res != 0 )
    {
      printf("ERR: failed to validate or install package dependences\n");
      return -1;
    }
  }
  
  if ( cmdlineswitches.nosoftwareinstall == 0 )
  {
    if ( downloadinstallnexfsbinaries() == -1 )
    {
      printf("ERR: failed to download nexfs solftware\n");
      return -1;
    }
  }

  if ( cmdlineswitches.nocheckbinaries == 0 )
  {
    if ( checkinstalledbinaries() == -1 )
    {
      printf("ERR: failed to download nexfs solftware\n");
      return -1;
    }
  }

  if ( cmdlineswitches.noserviceinstall ==  0 )
  {
    if ( installsystemdservice() == -1 )
    {
      printf("ERR: failed to install systemd nexfs.server start/stop script\n");
      return -1;
    }

  }
  if ( cmdlineswitches.noinit == 0 )
  {
    if ( init() == -1 )
    {
      printf("ERR: failed to init nexfs\n");
      return -1;
    }
  }
  else if ( cmdlineswitches.generateconfigfiles == 1 )
  {
    if ( generateconfigfiles() == -1 )
    {
      printf("ERR: failed to init nexfs\n");
      return -1;
    }
  }

  accepttermsandconditions();

  return 0;
}

int upgradenexfs()
{
  printf("Auto Upgrade not yet implemented\n");
  return 0;
}

int help(int argc, char *argv[])
{
  int NUMOFLEVELS=0;
  char *VALIDDEBUGLEVELSLABELS[] = {  "CRIT", "ERR", "WARNING", "NOTICE","INFO", "DEBUG" };
  char *VALIDSYSLOGFACILITY[] = { "LOG_USER", "LOG_LOCAL0", "LOG_LOCAL1", "LOG_LOCAL2", "LOG_LOCAL3", "LOG_LOCAL4", "LOG_LOCAL5", "LOG_LOCAL6", "LOG_LOCAL7" };
  int loop=0;

  if ( argc > 2 )
  {
    if ( strcmp(argv[2],"loglevels") == 0 )
    {
      NUMOFLEVELS=(sizeof(VALIDDEBUGLEVELSLABELS)/sizeof(VALIDDEBUGLEVELSLABELS[0]));

      printf("Valid Loglevels:\n");
      for ( loop=0; loop<NUMOFLEVELS; loop++)
      {
        printf("%s\n",VALIDDEBUGLEVELSLABELS[loop]);
      }
      printf("\n");
      return 0;
    }
    else if ( strcmp(argv[2],"syslogfacility") == 0 )
    {
      printf("Valid syslogfacilities:\n");
      NUMOFLEVELS=(sizeof(VALIDSYSLOGFACILITY)/sizeof(VALIDSYSLOGFACILITY[0]));

      for ( loop=0; loop<NUMOFLEVELS; loop++)
      {
        printf("%s\n",VALIDSYSLOGFACILITY[loop]);
      }
      printf("\n");
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

  printf("Use %s the manage the local Nexustorage Nexfs Server\n",argv[1]);
  printf("Usage %s COMMAND ARGS --allhelp --confdir=PATH\n",argv[0]);
  printf("  --allhelp, print extended help, requires no additional COMMAND or ARGS\n");
  printf("  --confdir, path to nexfs configuration directory, (optional) requried if not using the Nexfs default\n");
  printf("  The configuration dir can also be set by the enviromental var nexfsconfdir\n"); 
  printf("COMMAND and ARGS:");
  printf("  configfile get all [tagname]\n");
  printf("  configfile get taglabel [tagname]\n");
  printf("  configfile get value [tagname]\n");
  printf("  configfile get help [tagname]\n");
  printf("  configfile set [tagname] [newvalue]\n");
  printf("  configfile set defaults [system]  - where system is either minio or s3\n");
  printf("  configfile get loglevel\n");
  printf("  configfile dumpall\n");
  printf("  liveconfig get value [tagname]\n");
  printf("  liveconfig set [tagname] [newvalue]\n");
  printf("  liveconfig get loglevel\n");
  printf("  liveconfig set loglevel [newloglevel]\n");
  printf("  liveconfig get syslogfacility\n");
  printf("  liveconfig set syslogfacility [newsyslogfacility]\n");
  printf("  liveconfig dumpall\n");
  printf("  security admin get cmd group\n");
  printf("  security admin set cmd group [groupname or id]\n");
  printf("  security admin get config group\n");
  printf("  security admin set config group [groupname or id]\n");
  printf("  security admin get cmd access\n");
  printf("  security admin get config access\n");
  printf("  nfs getexports [optional output filename]\n");
  printf("  nfs putexports [optional import filename]\n");
  printf("  iscsi getconf [optional output filename]\n");
  printf("  iscsi putconf [optional import filename]\n");
  printf("  iscsi show targets\n");
  printf("  iscsi show target [targetid]\n");
  printf("  iscsi show interfaces\n");
  printf("  iscsi show sessions\n");
  printf("  iscsi show targetsessions [targetid]\n");
  printf("  iscsi stat targets\n");
  printf("  iscsi stat target [targetid]\n");
  printf("  iscsi stat sessions\n");
  printf("  iscsi stat session [sessionid]\n");
  printf("  iscsi stat connections\n");
  printf("  iscsi stat connection [connectid]\n");
  printf("  iscsi stat luns\n");
  printf("  iscsi stat lun [targetid] [lunid]\n");
  printf("  iscsi stat targetluns [targetid]\n");
  printf("  help loglevels\n");
  printf("  help syslogfacility\n");
  printf("  help configtags\n");
  printf("  help configtag [tagname]\n");
  printf("  release get nexfscli\n");
  printf("  release get nexfs\n");
  printf("  file info [filename (including nexfs path)] {-sid}\n");
  printf("  file tierusage [filename (including nexfs path)]\n");
  printf("  file extendedtierusage [filename (including nexfs path)]\n");
  printf("  file movetotier3 [filename (including path)]\n");
  printf("  jobqueue list\n");
  printf("  jobqueue list jobid [jobid]\n");
  printf("  server status\n");
  printf("  server start\n");
  printf("  server stop\n");
  printf("  server forcestop\n");
  printf("  server license\n");
  printf("  server stats\n");
  printf("  server counters\n");
  return 0;
}

int allhelp(int argc, char *argv[])
{
  printf("syncstructuredata\n");
  printf("init\n");
  printf("upgrade\n");
  printf("setupdatastores\n");
  printf("generateconfigfiles\n");
  printf("installsystemdservice\n");
  printf("accepttermsandconditions\n");
  printf("install --reinstall --configoverwrite --nosoftwareinstall --noserviceinstall --noinit --generateconfigfiles --nonfs --accepttermsandcondition --noinstallpackages --noprompt\n");
  printf("file info -set [filename (including nexfs path)] attrib newvalue\n");

  help(argc,argv);

  return 0;
}

int main(int argc, char *argv[])
{
  int res=0;

  MYNAME=argv[0];
  GFSCONFDIR=NULL;

  if ( check_args(argc, 2, GREATER )) return -1;

  if ( strcmp(argv[1],"--allhelp") == 0 )
  {
    allhelp(argc, argv);
    return 0;
  } 

  if (  process_argvswitches(&argc,argv) == -1 )
    return -1;

  if (GFSCONFDIR==NULL)
  {
    if ( (GFSCONFDIR=getenv( "nexfsconfdir" )) == NULL )
    {
      GFSCONFDIR=malloc(strlen(DEFAULTGFSCONFDIR)+1);
      snprintf(GFSCONFDIR,strlen(DEFAULTGFSCONFDIR)+1,"%s",DEFAULTGFSCONFDIR);
    }
  }

  if ( strcmp(argv[1],"help") == 0 )
  {
    help(argc, argv);
    return 0;
  }
  else if ( strcmp(argv[1],"generateconfigfiles") == 0 )
  {
    res=generateconfigfiles(argv);
    return res;
  }
  else if ( strcmp(argv[1],"installsystemdservice") == 0 )
  {
    res=installsystemdservice();
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
  else if (( strcmp(argv[1],"init") == 0 ) || ( strcmp(argv[1],"-init") == 0 )) 
  {
    res=init();
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
  else if ( strcmp(argv[1],"nfs") == 0 )
  {
    res=nfs(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"iscsi") == 0 )
  {
    res=iscsi(argc, argv);
    return res;
  }
  else if ( strcmp(argv[1],"accepttermsandconditions") == 0 )
  {
    res=accepttermsandconditions();
    return res;
  }
  else if ( strcmp(argv[1],"install") == 0 )
  {
    res=installnexfs();
    return res;
  }
  else if ( strcmp(argv[1],"upgrade") == 0 )
  {
    res=upgradenexfs();
    return res;
  }
  else if ( strcmp(argv[1],"syncstructuredata") == 0 )
  {
    res=syncstructurefilesystems(argc, argv);
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
