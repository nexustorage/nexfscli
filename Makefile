CC=gcc
CFLAGS= -Wall -O0 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto  -lm  -luuid -ggdb -fno-stack-protector -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
#CFLAGS= -Wall -O3 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto   -luuid -ggdb -fno-stack-protector 
#CFLAGS= -Wall -O3 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto   -lulockmgr -luuid -ggdb -fno-stack-protector 
#CFLAGS= -Wall -O3  -lcurl  -lcrypto   -lulockmgr -luuid -ggdb -fno-stack-protector -D_FILE_OFFSET_BITS=64

#nexfs.server: gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_lowlevel.o 
#	$(CC) -o nexfs.server gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_lowlevel.o $(CFLAGS)

nexfs.server: gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o 
	$(CC) -o nexfs.server gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o $(CFLAGS)

gfs_configkeynamesum: gfs_configkeynamesum.o gfsconf.o logging.o 
	$(CC) -o gfs_configkeynamesum gfs_configkeynamesum.o gfsconf.o  logging.o $(CFLAGS)

nexfscli: nexfscli.o gfsconf.o gfslogging.o
	$(CC)  -o nexfscli  nexfscli.o gfsconf.o gfslogging.o  $(CFLAGS)  

clean: 
	rm gfserv.o logging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfs_configkeynamesum.o gfsjobs.o gfstiering.o nexfscli.o gfsdeletions.o gfsfuse_lowlevel.o

#gcc -Wall gfserv.c logging.c -I/usr/include/fuse3  -lfuse3 -lpthread   -lulockmgr -o gfserv
