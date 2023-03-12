/*
Used the chardev.c from bash session 6 as skeleton! 
*/
// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>
MODULE_LICENSE("GPL");

//Our custom definitions of IOCTL operations
#include "message_slot.h"

static Cnode * slots [MAX_MINOR];
// The message the device will give when asked

//=================== HELPERS ========================================

static Cnode * getChannel(unsigned long slot,unsigned long channel){
  /*
  return the Cnode of the Channel asked for
  or NULL if not exist
  */
 Cnode * base= slots[slot];
 while(base != NULL){
  if (base ->channel == channel){
    return base;
  }
  base = base ->next;
 }
 return NULL;
}
static Cnode * addChannel(unsigned long slot,unsigned long channel){
  /*
  create and return new Cnode in the slot provided
  return NULL if error
  */
  Cnode * base= slots[slot];
  if(base!=NULL){
    Cnode * next = base ->next;
    base -> next= kmalloc(sizeof(Cnode),GFP_KERNEL);
    (base -> next) -> next = next;
    (base -> next) -> channel = channel;
    return base->next;
  }
  return NULL;
}
static void clean_slot(Cnode *base){
  if (base==NULL){
    return;
  }
  clean_slot(base -> next);
  if (base -> the_message !=NULL){
    kfree (base -> the_message);
  }
  kfree(base);
}

static void clean_all(void){
  int i;
  for(i=0 ; i<MAX_MINOR ; i++ ){
    clean_slot(slots[i]);
  }
}


//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
  unsigned long minor = iminor(inode);
  if (slots[minor]==NULL){//so its the first time this slot open 
    slots[minor]= kmalloc(sizeof(Cnode),GFP_KERNEL);
    slots[minor] ->channel = 0;//first node will always be invalid channel
  }
  
  return SUCCESS;
}

//---------------------------------------------------------------
static int device_release( struct inode* inode,
                           struct file*  file)
{
  return SUCCESS;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
  if ((file -> private_data)==NULL){
    return -EINVAL;
  }
  unsigned long minor = iminor(file -> f_inode);
  unsigned long channelnum= (unsigned long)(file -> private_data);
  Cnode * channel = getChannel(minor,channelnum);
  if (channel == NULL || (channel ->the_message)==NULL){
      return -EWOULDBLOCK;
    }
  int i;
  int size= channel ->size;
  if(length<size){
    return  -ENOSPC;
  }
  for(i=0;i<size;i++){
    if(put_user((channel->the_message)[i],buffer+i)!= 0){
      return -EINVAL;//TODO: check what to return
    }
  }
  return i;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
  if ((file -> private_data)==NULL){
    return -EINVAL;
  }
  if( length==0 || length>BUF_LEN){
    return -EMSGSIZE;
  }
  unsigned long channelnum= (unsigned long)(file -> private_data);
  unsigned long minor = iminor(file -> f_inode);
  Cnode * channel = getChannel(minor,channelnum);
  if (channel == NULL){//so we need to add this channel
    channel = addChannel(minor,channelnum);
  }
  int i;
  char * new_message = kmalloc(sizeof(char)*length,GFP_KERNEL);
  for(i = 0; i < length; i++) {
    if(get_user(new_message[i], buffer+i) != 0){
      kfree(new_message);
      return -EINVAL;//return error
    }
  }
    if (i==length){
      if((channel->the_message)!= NULL){
        kfree(channel->the_message);
      }
        channel->the_message = new_message;
        channel ->size=i;
        return i;
    }
    kfree(new_message);
    return -EINVAL;//return some error 
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
  if( MSG_SLOT_CHANNEL != ioctl_command_id || ioctl_param == 0) {
    return -EINVAL;
  }
  (file -> private_data) = ioctl_param;
  return SUCCESS;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
  int rc = -1;
  // init dev struct

  // Register driver capabilities. Obtain major num
 
  rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
  // Negative values signify an error
  if( rc < 0 ) {
    
    printk( KERN_ALERT "%s registraion failed for  %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM );
    return rc;
  }

  return 0;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
  // Unregister the device
  // Should always succeed
  clean_all(); 
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
