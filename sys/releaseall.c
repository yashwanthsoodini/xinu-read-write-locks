/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int isnotheld(int ldes);
LOCAL void remplock(int ldes);
LOCAL void wakeup(int ldes);

/* releaseall - release all the specified locks, waking up one waiting process */

SYSCALL releaseall(int numlocks, int locks)
{
    STATWORD ps;
    register struct lentry *lptr;
    int *ldes;
    int retval = OK;

    struct lentry *lptr;

    disable(ps);
    for( ; numlocks>0; numlocks--){
        ldes = (int *)(&locks) + (numlocks-1);
        remplock(*ldes);
        if (isbadlock(*ldes) || isnotheld(*ldes))
        {
            retval = SYSERR;
        } else
        {
            lptr = &locktab[*ldes];
            lptr->lholdtype = NONE;
            wakeupwp(*ldes);
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

LOCAL void remplock(int ldes)
{
    struct pentry *pptr;
    pptr = &proctab[currpid];

    locknode *curr = pptr->plocks;

    if (curr!=NULL && curr->ldes == ldes)
    {
        pptr->plocks = curr->next;
        free(curr);
        return;
    }

    locknode *prev;
    while (curr!=NULL && curr->ldes!=ldes)
    {
        prev = curr;
        curr = curr->next;
    }
    
    if(curr == NULL) return;

    prev->next = curr->next;
    free(curr);
    return;
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
                    struct qent *mptr;
                    mptr = &q[pid];
                    int temp = mptr->qnext;
                    q[mptr->qprev].qnext = mptr->qnext;
                    q[mptr->qnext].qprev = mptr->qprev;
                    pptr->pwaitlock = -1;
                    pptr->plwaittype = NONE;
                    ready(pid, RESCHNO);
                    pid = temp;
                    pptr = &proctab[pid];
                }
                lptr->lprio = q[q[lptr->lqhead].qnext].qkey;
                return;
            }
            break;
        }
        pid = q[pid].qnext;
    }
    ready(getfirst(lptr->lqhead), RESCHNO);
    return;
}