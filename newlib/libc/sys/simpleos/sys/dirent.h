#ifndef _SYS_DIRENT_H_
#define	_SYS_DIRENT_H_

#include <stdint.h>

#define MAXNAMLEN 260

typedef struct DIR DIR;

struct dirent {
    uint64_t d_ino;
    char d_name[MAXNAMLEN+1];
};


#endif