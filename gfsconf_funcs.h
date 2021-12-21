size_t gfs_configkeynamesum(char *);
int gfs_liveconf(char *, int , char *, size_t, off_t);
int gfs_createdefaultconf(char *, char *);
int gfs_getconfig(int, char *, char *, int, int);
int gfs_loadconfig(int);
int gfs_updateconfigfile(int, char *, void *, int);
int gfs_validateconfvalue(char *, char *, int);
int gfs_updateliveconf(char *,char *, size_t, off_t);
int get_setlowesttier();
int set_demigrationtierorder();


