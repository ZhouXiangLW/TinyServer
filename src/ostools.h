#ifndef OS_TOOLS
#define OS_TOOLS
#include <sys/sem.h>

/*******************信号量的PV操作**********************/
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short int * array;
    struct seaminfo * __buf;
};

int p(int sem_id);
int v(int sem_id);
void sem_set_val(int sem_id, int val);
/*******************信号量的PV操作**********************/

#endif