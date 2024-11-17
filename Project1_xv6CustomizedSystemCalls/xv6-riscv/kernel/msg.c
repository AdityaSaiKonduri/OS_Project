// kernel/msg.c
#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "msg.h"
#include "syscall.h"

struct msgtable msgtable;

static void
k2u_msqid_ds(struct msg_queue *kds, struct msqid_ds *uds)
{
    uds->msg_perm = 0; 
    uds->msg_qnum = kds->msg_count;
    uds->msg_qbytes = kds->queue_size;
    uds->msg_lspid = 0;
    uds->msg_lrpid = 0;
    uds->msg_stime = 0;
    uds->msg_rtime = 0;
    uds->msg_ctime = 0;
}

static void
u2k_msqid_ds(struct msqid_ds *uds, struct msg_queue *kds)
{
    kds->queue_size = uds->msg_qbytes;
}

void
msginit(void)
{
    initlock(&msgtable.lock, "msgtable");
    for(int i = 0; i < MSG_MAX_QUEUES; i++) {
        initlock(&msgtable.queues[i].lock, "msgqueue");
        msgtable.queues[i].in_use = 0;
        msgtable.queues[i].msg_count = 0;
        msgtable.queues[i].first_msg = 0;
        msgtable.queues[i].last_msg = 0;
        msgtable.queues[i].queue_size = MSG_MAX_SIZE;
    }
}

uint64
sys_msgget(void)
{
  int key, flags;
  if(argint(0, &key) < 0 || argint(1, &flags) < 0)
    return -1;

  acquire(&msgtable.lock);
  for(int i = 0; i < MSG_MAX_QUEUES; i++) {
    if(!msgtable.queues[i].in_use) {
      msgtable.queues[i].in_use = 1;
      release(&msgtable.lock);
      return i;
    }
  }
  release(&msgtable.lock);
  return -1;
}

uint64
sys_msgsnd(void)
{
  int msqid;
  uint64 msgp;
  int msgsz, flags;

  if(argint(0, &msqid) < 0 || argaddr(1, &msgp) < 0 ||
     argint(2, &msgsz) < 0 || argint(3, &flags) < 0)
    return -1;

  if(msqid < 0 || msqid >= MSG_MAX_QUEUES || !msgtable.queues[msqid].in_use)
    return -1;

  struct msg_buf msg;
  if(copyin(myproc()->pagetable, (char*)&msg, msgp, sizeof(struct msg_buf)) < 0)
    return -1;

  if(msgsz < 0 || msgsz > MSG_MAX_SIZE)
    return -1;

  acquire(&msgtable.queues[msqid].lock);
  
  if(msgtable.queues[msqid].msg_count >= msgtable.queues[msqid].queue_size) {
    if(flags & IPC_NOWAIT) {
      release(&msgtable.queues[msqid].lock);
      return -1;
    }
    // Implement sleeping wait
    while(msgtable.queues[msqid].msg_count >= msgtable.queues[msqid].queue_size) {
      sleep(&msgtable.queues[msqid], &msgtable.queues[msqid].lock);
    }
  }

  int msg_index = msqid * MSG_MAX_SIZE + msgtable.queues[msqid].last_msg;
  msgtable.messages[msg_index] = msg;

  msgtable.queues[msqid].last_msg = (msgtable.queues[msqid].last_msg + 1) % MSG_MAX_SIZE;
  msgtable.queues[msqid].msg_count++;

  wakeup(&msgtable.queues[msqid]);
  release(&msgtable.queues[msqid].lock);
  return 0;
}

uint64
sys_msgrcv(void)
{
  int msqid;
  uint64 msgp;
  int msgsz, flags;
  long msgtyp;

  if(argint(0, &msqid) < 0 || argaddr(1, &msgp) < 0 ||
     argint(2, &msgsz) < 0 || argint(3, (int*)&msgtyp) < 0 || 
     argint(4, &flags) < 0)
    return -1;

  if(msqid < 0 || msqid >= MSG_MAX_QUEUES || !msgtable.queues[msqid].in_use)
    return -1;

  if(msgsz < 0 || msgsz > MSG_MAX_SIZE)
    return -1;

  acquire(&msgtable.queues[msqid].lock);

  if(msgtable.queues[msqid].msg_count == 0) {
    if(flags & IPC_NOWAIT) {
      release(&msgtable.queues[msqid].lock);
      return -1;
    }
    // Could implement sleeping wait here
    release(&msgtable.queues[msqid].lock);
    return -1;
  }

  int msg_index = msqid * 100 + msgtable.queues[msqid].first_msg;
  struct msg_buf *msg = &msgtable.messages[msg_index];

  // Type matching logic
  if(msgtyp > 0 && msg->mtype != msgtyp) {
    release(&msgtable.queues[msqid].lock);
    return -1;
  }
  
  if(copyout(myproc()->pagetable, msgp, (char*)msg, sizeof(struct msg_buf)) < 0) {
    release(&msgtable.queues[msqid].lock);
    return -1;
  }

  msgtable.queues[msqid].first_msg = (msgtable.queues[msqid].first_msg + 1) % 100;
  msgtable.queues[msqid].msg_count--;

  release(&msgtable.queues[msqid].lock);
  return msgsz;
}

uint64
sys_msgctl(void)
{
    int msqid, cmd;
    uint64 buf_addr;
    struct msg_queue *queue;
    struct msqid_ds buf;

    if(argint(0, &msqid) < 0 || argint(1, &cmd) < 0 || argaddr(2, &buf_addr) < 0)
        return -1;

    // Check if msqid is valid
    if(msqid < 0 || msqid >= MSG_MAX_QUEUES)
        return -1;

    queue = &msgtable.queues[msqid];

    acquire(&queue->lock);

    switch(cmd) {
        case IPC_STAT:
            if(!queue->in_use) {
                release(&queue->lock);
                return -1;
            }
            k2u_msqid_ds(queue, &buf);
            if(copyout(myproc()->pagetable, buf_addr, (char*)&buf, sizeof(buf)) < 0) {
                release(&queue->lock);
                return -1;
            }
            break;

        case IPC_SET:
            if(!queue->in_use) {
                release(&queue->lock);
                return -1;
            }
            if(copyin(myproc()->pagetable, (char*)&buf, buf_addr, sizeof(buf)) < 0) {
                release(&queue->lock);
                return -1;
            }
            u2k_msqid_ds(&buf, queue);
            break;

        case IPC_RMID:
            if(!queue->in_use) {
                release(&queue->lock);
                return -1;
            }
            queue->in_use = 0;
            queue->msg_count = 0;
            queue->first_msg = 0;
            queue->last_msg = 0;
            break;

        default:
            release(&queue->lock);
            return -1;
    }

    release(&queue->lock);
    return 0;
}