/* lock.c - acquire lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int isdeleted(int ldes);
LOCAL int wrwaiting(int ldes, int wprio);

/* lock - make current process acquire a lock */
SYSCALL lock(int ldes, int type, int priority)
{
    STATWORD ps;
    struct lentry *lptr;
    struct pentry *pptr;

    disable(ps);
    if(isbadlock(ldes) || (lptr = &locktab[ldes])->lstate==LFREE || isdeleted(ldes)){
        restore(ps);
        return(SYSERR);
    }

    if((type==READ && lptr->lholdtype==WRITER || (lptr->lholdtype==READER && wrwaiting())) || type==WRITE && lptr->lholdtype!=NONE){
        (pptr = &proctab[currpid])->pstate = PRWAIT;
        pptr->pwaitlock = ldes;
        insert(currpid, lptr->lqhead, priority);
        if(priority > lptr->lprio)
            lptr->lprio = priority;
        pptr->pwaitret = OK;
        resched();
        restore(ps);
        return pptr->pwaitret;
    }
    restore(ps);
    return(OK);
}

LOCAL int isdeleted(int ldes)
{
    struct pentry *pptr;
    pptr = &proctab[currpid];

    locknode *lock;
    lock = pptr->pdellocks;
    while (lock!=NULL)
    {
        if (lock->ldes==ldes)
            return TRUE;
        lock = lock->next;
    }
    return FALSE;
}

LOCAL int wrwaiting(int ldes, int wprio)
{
    struct lentry *lptr;
    lptr = &locktab[ldes];

    int pid = q[lptr->lqhead].qnext;
    while (pid != lptr->lqtail)
    {   
        if (q[pid].qlwaittype == WAITWRITE && q[pid].qkey > wprio)
            return TRUE;
    }
    return FALSE;
}