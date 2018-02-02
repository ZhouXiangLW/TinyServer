#include "ostools.h"

int p(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    return semop(sem_id, &sem_b, 1);
}

int v(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    return semop(sem_id, &sem_b, 1);
}

void sem_set_val(int sem_id, int val)
{
    union semun sem_un;
    sem_un.val = val;
    semctl(sem_id, 0, SETVAL, sem_un);
}