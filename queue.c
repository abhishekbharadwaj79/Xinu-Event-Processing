/* queue.c - enqueue, dequeue */

#include <xinu.h>

struct qentry	queuetab[NQENT];	/* Table of process queues	*/

/*------------------------------------------------------------------------
 *  enqueue  -  Insert a process at the tail of a queue
 *------------------------------------------------------------------------
 */
pid32	enqueue(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q		/* ID of queue to use		*/
	)
{
	qid16	tail, prev;		/* Tail & previous node indexes	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	tail = queuetail(q);
	prev = queuetab[tail].qprev;

	queuetab[pid].qnext  = tail;	/* Insert just before tail node	*/
	queuetab[pid].qprev  = prev;
	queuetab[prev].qnext = pid;
	queuetab[tail].qprev = pid;
	return pid;
}

/*------------------------------------------------------------------------
 *  dequeue  -  Remove and return the first process on a list
 *------------------------------------------------------------------------
 */
pid32	dequeue(
	  qid16		q		/* ID queue to use		*/
	)
{
	pid32	pid;			/* ID of process removed	*/

	if (isbadqid(q)) {
		return SYSERR;
	} else if (isempty(q)) {
		return EMPTY;
	}

	pid = getfirst(q);
	queuetab[pid].qprev = EMPTY;
	queuetab[pid].qnext = EMPTY;
	return pid;
}


void enqueueProc(topic16 topic, uint32 data) {
    if(front == (rear + 1) || (front == 0 && rear == (QUEUE_SIZE -1))){
        return;
    }
    else {
        if(front == -1) {
            front = 0;
            rear = 0;
        }
        else if(rear == (QUEUE_SIZE - 1)) {
            rear = 0;
        }
        else {
            rear += 1;
        }
        pendingQueue[rear].data = data;
        pendingQueue[rear].topic = topic;
    }
}

void dequeueProc() {
    if(isQueueEmpty()){
        return;
    }
    else {
        if(front == rear) {
            front = -1;
            rear = -1;
        }
        else if(front == (QUEUE_SIZE - 1)){
            front = 0;
        }
        else {
            front += 1;
        }
    }
}

bool8 isQueueEmpty() {
    if(front == -1) {
        return TRUE;
    }
    return FALSE;
}