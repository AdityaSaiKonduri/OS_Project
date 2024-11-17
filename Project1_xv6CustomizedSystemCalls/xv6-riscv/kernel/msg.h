#ifndef KERNEL_MSG_H
#define KERNEL_MSG_H

#include "spinlock.h"
#include "types.h"

// Constants
#define MSG_MAX_QUEUES 16
#define MSG_MAX_SIZE   1024
#define MSG_NOERROR    0

// IPC permission
#define MSG_R    0400  // Read permission
#define MSG_W    0200  // Write permission

// msgget flags
#define IPC_CREAT  01000  // Create entry if key doesn't exist
#define IPC_EXCL   02000  // Fail if key exists
#define IPC_NOWAIT 04000  // Return error on wait

// msgctl commands
#define IPC_RMID 0      // Remove identifier
#define IPC_SET  1      // Set options
#define IPC_STAT 2      // Get options

// Message structure for the queue
struct msg_buf {
  long mtype;          // Message type
  char mtext[MSG_MAX_SIZE];    // Message text
};

// Message queue DS structure (same as user space)
struct msqid_ds {
    int msg_perm;    // Ownership and permissions
    uint msg_qnum;    // Number of messages currently on queue
    uint msg_qbytes;  // Maximum number of bytes allowed on queue
    int msg_lspid;   // Process ID of last msgsnd()
    int msg_lrpid;   // Process ID of last msgrcv()
    uint msg_stime;   // Time of last msgsnd()
    uint msg_rtime;   // Time of last msgrcv()
    uint msg_ctime;   // Time of last change
};

// Queue control structure
struct msg_queue {
  int in_use;          // Is this queue being used?
  int msg_count;       // Number of messages in queue
  int first_msg;       // Index of first message
  int last_msg;        // Index of last message
  int queue_size;      // Size of message queue
  struct spinlock lock;// Lock for the queue
};

// Global message table structure
extern struct msgtable {
    struct spinlock lock;
    struct msg_queue queues[MSG_MAX_QUEUES];
    struct msg_buf messages[MSG_MAX_QUEUES * 100];
} msgtable;

// System call declarations
int msgget(int key, int msgflg);
int msgsnd(int msqid, const struct msg_buf *msgp, int msgsz, int msgflg);
int msgrcv(int msqid, struct msg_buf *msgp, int msgsz, long msgtyp, int msgflg);
int msgctl(int msqid, int cmd, struct msg_queue *buf);

// Function declarations
void msginit(void);

#endif