/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/* releaseall - release all the specified locks, waking up one waiting process */

SYSCALL releaseall(int numlocks, int locks)
{
    STATWORD ps;
    register struct lentry *lptr;

    disable(ps);
}