  /*
use user_chardev.c from bash ssesion 6 as skeleton!!!!!
*/
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h"

int main(int argc,char *argv[])
{
  int file_desc;
  int ret_val;
  if (argc != 3){
    perror("Error : ");
    return 1;
  }

  file_desc = open( argv[1], O_RDWR );
  if( file_desc < 0 ) {
    perror("Error : ");
    return 1;
  }
  int channel = atoi(argv[2]);
  ret_val = ioctl( file_desc, MSG_SLOT_CHANNEL, channel);
  if(ret_val<0){
    perror("Error : ");
    return 1;
  }
  char * buff = malloc(sizeof(char)*128);
  ret_val = read(file_desc, buff, 128);
  if (ret_val>0){
    buff=realloc(buff,ret_val*sizeof(char));
  }
  else{
    perror("Error : ");
    return 1;
  }
  close(file_desc); 
  ret_val = write(1,buff,ret_val);
  if(ret_val<0){
    perror("Error : ");
    return 1;
  }
  free(buff);
  return 0;
}
