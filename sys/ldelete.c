/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL void addtopdellocks(int pid, int ldes);

/* ldelete -- delete a lock by releasing its table entry */
SYSCALL ldelete(int ldes)
{      
    STATWORD ps;
    int pid;
    struct lentry *lptr;
    
    disable(ps);
    if(isbadlock(ldes) || locktab[ldes].lstate==LFREE){
        restore(ps);
        return(SYSERR);
    }

    lptr = &locktab[ldes];
    lptr->lstate = LFREE;

    procnode *proc = lptr->lprocs;
    procnode *temp;
    while (proc!=NULL)
    {
        addtopdellocks(proc->pid, ldes);
        temp = proc;
        proc = proc->next;
        free(temp);
    }

    lptr->lprocs = NULL;
    lptr->lholdtype = NONE;
    lptr->lprio = -1;

    struct pentry *pptr;
    
    if(nonempty(lptr->lqhead)){
        while( (pid=getfirst(lptr->lqhead)) != EMPTY)
        {
            pptr = &proctab[pid];
            pptr->pwaitret = DELETED;
            pptr->pwaitlock = -1;
            pptr->plwaittype = NONE;
            ready(pid, RESCHNO);
        }
        resched();
    }
    restore(ps);
    return(OK);
}

LOCAL void addtodellocks(int pid, int ldes)
{
    struct pentry *pptr;
    pptr = &proctab[pid];
    locknode *lock = (locknode *)malloc(sizeof(locknode));
    lock->ldes = ldes;
    lock->next = pptr->pdellocks;
    pptr->pdellocks = lock;
    return;
}