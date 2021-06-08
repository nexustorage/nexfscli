CC=gcc
CFLAGS= -Wall -O0 `pkg-config fuse3 --cflags --libs` -lcurl  -lcrypto  -lm  -luuid -ggdb -fno-stack-protector -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

nexfscli: nexfscli.o gfsconf.o gfslogging.o
	$(CC)  -o nexfscli  nexfscli.o gfsconf.o gfslogging.o  $(CFLAGS)  

clean: 
	rm nexfscli.o

