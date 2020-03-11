/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int isnotheld(int ldes);
LOCAL void wakeup(int ldes);

/* releaseall - release all the specified locks, waking up one waiting process */

SYSCALL releaseall(int numlocks, int locks)
{
    STATWORD ps;
    register struct lentry *lptr;
    int *lock;
    int retval = OK;

    disable(ps);
    for( ; numlocks>0; numlocks--){
        lock = (int *)(&locks) + (numlocks-1);
        if (isbadlock(lock) || isnotheld(lock))
        {
            retval = SYSERR;
        } else
        {
            wakeupwp(lock);
        }
    }
    resched();
    restore(ps);
    return(retval);
}

LOCAL int isnotheld(int ldes)
{
    struct pentry *pptr;
    pptr = &proctab[currpid];

    locknode *lock;
    lock = pptr->plocks;
    while (lock!=NULL)
    {
        if (lock->ldes==ldes)
            return FALSE;
        lock = lock->next;
    }
    return TRUE;
}

LOCAL void wakeup(int ldes)
{
    struct lentry *lptr;
    lptr = &locktab[ldes];
    int lprio = lptr->lprio;

    struct pentry *pptr;

    int pid = q[lptr->lqhead].qnext;
    unsigned long wst = q[pid].qwst;

    while (pid != lptr->lqtail && q[pid].qkey==lprio)
    {
        pptr = &proctab[pid];

        if (pptr->plwaittype == READ)
        {
            if (wst - q[pid].qwst <= 400)
            {
                while (pid != lptr->lqtail && pptr->plwaittype == READ)
                {
                    ready(pid, RESCHNO);
                    pid = q[pid].qnext;
                    pptr = &proctab[pid];
                }
                return;
            }
            break;
        }
        pid = q[pid].qnext;
    }
    ready(q[lptr->lqhead].qnext, RESCHNO);
    return;
}