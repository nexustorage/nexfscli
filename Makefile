CC=gcc
DEBUGCFLAGS= -Wall -O0 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto  -lm  -luuid -ggdb -fno-stack-protector -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lz
CFLAGS= -Wall -O3 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto  -lm  -luuid -ggdb -fno-stack-protector -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lz
#CFLAGS= -Wall -O3 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto   -luuid -ggdb -fno-stack-protector 
#CFLAGS= -Wall -O3 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto   -lulockmgr -luuid -ggdb -fno-stack-protector 
#CFLAGS= -Wall -O3  -lcurl  -lcrypto   -lulockmgr -luuid -ggdb -fno-stack-protector -D_FILE_OFFSET_BITS=64

#nexfs.server: gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_lowlevel.o 
#	$(CC) -o nexfs.server gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_lowlevel.o $(CFLAGS)

nexfs.server-debug: gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_writefd.o gfsencryptcompress.o gfslicense.o 
	$(CC) -o nexfs.server-debug gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_writefd.o gfsencryptcompress.o gfslicense.o $(DEBUGCFLAGS)

nexfs.server: gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_writefd.o gfsencryptcompress.o gfslicense.o 
	$(CC) -o nexfs.server gfserv.o gfslogging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfstiering.o  gfsjobs.o gfsdeletions.o gfsfuse_writefd.o gfsencryptcompress.o gfslicense.o $(CFLAGS)

gfs_configkeynamesum: gfs_configkeynamesum.o gfsconf.o logging.o 
	$(CC) -o gfs_configkeynamesum gfs_configkeynamesum.o gfsconf.o  logging.o $(CFLAGS)

nexfscli: nexfscli.o gfsconf.o gfslogging.o
	$(CC)  -o nexfscli  nexfscli.o gfsconf.o gfslogging.o  $(CFLAGS)  

nexfslicensegen: nexfslicencekeygen.o  gfss3sign.o gfslicense.o
	$(CC)  -o nexfslicencekeygen nexfslicencekeygen.o  gfss3sign.o gfslicense.o  $(CFLAGS) 

nexfslicensevalidate: nexfslicensevalidate.o  gfss3sign.o gfslicense.o
	$(CC)  -o nexfslicensevalidate nexfslicensevalidate.o  gfss3sign.o gfslicense.o  $(CFLAGS)  

clean: 
	rm gfserv.o logging.o gfsfuse.o gfsfuncs.o gfsdatafile.o gfscurl.o gfss3sign.o gfstat.o gfsdebug.o gfsconf.o gfs_configkeynamesum.o gfsjobs.o gfstiering.o nexfscli.o gfsdeletions.o gfsencryptcompress.o gfsfuse_writefd.o  gfslogging.o  gfss3sign.o gfslicense.o

#gcc -Wall gfserv.c logging.c -I/usr/include/fuse3  -lfuse3 -lpthread   -lulockmgr -o gfserv
