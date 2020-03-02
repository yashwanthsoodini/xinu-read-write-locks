/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/* ldelete -- delete a lock by releasing its table entry */
SYSCALL ldelete(int lock)
{
    STATWORD ps;
    int pid;
    struct lentry *lptr;

    disable(ps);
    if(isbadlock(lock) || locktab[lock].lstate==LFREE){
        restore(ps);
        return(SYSERR);
    }
    lptr = &locktab[lock];
    lptr->lstate = LFREE;
    if(nonempty(lptr->lqhead)){
        while( (pid=getfirst(lptr->lqhead)) != EMPTY)
        {
            proctab[pid].pwaitret = DELETED;
            ready(pid, RESCHNO);
        }
        resched();
    }
    restore(ps);
    return(OK);
}