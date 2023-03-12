/*
Used the chardev.h from bash session 6 as skeleton! 
*/

#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

// The major device number.
// We don't rely on dynamic registration
// any more. We want ioctls to know this
// number at compile time.
#define MAJOR_NUM 235

// Set the message of the device driver
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "char_dev"
#define BUF_LEN 128
#define MAX_MINOR 256
#define DEVICE_FILE_NAME "slot"
#define SUCCESS 0

struct node{
    unsigned long channel;
    char * the_message;
    int size;
    struct node * next;

};
typedef struct node Cnode;
struct chardev_info {
  unsigned long CURRENT_SLOT;
  Cnode * slots [MAX_MINOR];
};
#endif