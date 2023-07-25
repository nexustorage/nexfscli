CC=gcc
CFLAGS= -Wall -O3 `pkg-config fuse3 --cflags --libs` -fcommon -lcurl  -lcrypto  -lm  -luuid -ggdb -fno-stack-protector -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lz

nexfscli: nexfscli.o gfsconf_common.o
	$(CC)  -o nexfscli  nexfscli.o gfsconf_common.o   $(CFLAGS)

clean: 
	rm nexfscli.o 

