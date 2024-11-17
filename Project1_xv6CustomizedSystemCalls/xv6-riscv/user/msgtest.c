#include "kernel/types.h"
#include "kernel/msg.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"  // for IPC constants

#define MSGMAX 128
#define KEY 12345  // Using a fixed key instead of IPC_PRIVATE

int
main(int argc, char *argv[])
{
  struct msg_buf msg;  // Message buffer for sending
  
  // Initialize message type
  msg.mtype = 1;
  
  // Copy message text (using byte-by-byte copy since we don't have string.h)
  char *text = "Hello from xv6!";
  char *dest = msg.mtext;
  char *src = text;
  while((*dest++ = *src++) != 0)
    ;
  
  // Create a message queue
  int msqid = msgget(KEY, O_CREATE | 0666);
  if (msqid < 0) {
      printf("msgget failed\n");
      exit(1);
  }

  printf("msgget successful!\n");

  // // Send message
  // if(msgsnd(msqid, &msg, 14 /* length of "Hello from xv6!" */, 0) < 0) {
  //   printf("msgsnd failed\n");
  //   exit(1);
  // }

  // printf("msgsnd...");
  
  // // Receive message
  // memset(&msg, 0, sizeof(msg));  // Clear the buffer before receiving
  // if(msgrcv(msqid, &msg, MSGMAX, 1, 0) < 0) {
  //   printf("msgrcv failed\n");
  //   exit(1);
  // }

  // printf("msgrcv...");
  
  // printf("Received message: %s\n", msg.mtext);
  
  // // Remove message queue
  // if(msgctl(msqid, IPC_RMID, 0) < 0) {
  //   printf("msgctl failed\n");
  //   exit(1);
  // }

  // printf("msgctl...");
  // exit(0);
}