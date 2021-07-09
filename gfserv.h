#include "gfsconf.h"

#define RELEASETYPEDEV 0
#define RELEASETYPEBETA 1
#define RELEASETYPEPREVIEW 2
#define RELEASETYPEGA 3

#define NEXFSRELEASETYPE RELEASETYPEPREVIEW 


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

char *MYNAME;

