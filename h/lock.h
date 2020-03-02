/* lock.h - READ, WRITE, lcreate, ldelete, releaseall */

#ifndef _LOCK_H_
#define _LOCL_H_

#ifndef NLOCKS
#define NLOCKS  50  /* number of locks, if not defined */
#endif

#define READ    1   /* lock type read */
#define WRITE   2   /* lock type write */

#define	LFREE	'\01'		/* this semaphore is free		*/
#define	LUSED	'\02'		/* this semaphore is used		*/

#define READER  1   /* lock holder type reader */
#define WRITER  2   /* lock holder type writer */

typedef struct proclist{
    int pid;
    struct proclist *next;
} procnode;

struct  lentry  {       /* lock table entry                     */
    char lstate;        /* the state LFREE or LUSED             */
    int lqhead;         /* q index of head of wait list         */
    int lqtail;         /* q index of tail of wait list         */
    int lprio;          /* max priority among waiting processes */
    procnode *lholders; /* head of lock holders list            */
    int lholdtype;      /* the type READER or WRITER            */
};
extern struct lentry locktab[];
extern int nextlock;

#define isbadlock(l) (l<0 || l>=NLOCKS)

#endif

#include <kernel.h>

SYSCALL lcreate(void)
SYSCALL ldelete(int lock)
SYSCALL lock(int lock, int type, int priority)
SYSCALL releaseall(int numlocks, int lock1, ...)