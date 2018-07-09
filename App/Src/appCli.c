#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cli.h>
#include <errno.h>
#include <appCli.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define DEVICE_FILE_NAME "/dev/buffDevice" 

/* (10 * PAGE_SIZE) in accordance with Driver Virtual Buffer Size */
#define MAX 40960

int fd;
uint8_t rdbuff[MAX];
uint8_t wrbuff[MAX];

extern int quitFlag;

int openFile(int argc,char *argv[])
{
  fd = open(DEVICE_FILE_NAME, O_RDWR);
  if(fd < 0)
  {
    perror("Open Failed : ");
    return FAILURE;
  }
  printf("File Open Success\n");
  return SUCCESS;
}

int readFile(int argc,char *argv[])
{
  int retval,ii,size = 0;

  if(argc != 2)
  {
    printf("Usage <Cmd> <Size>\n");
    return FAILURE;
  }
  sscanf(argv[1],"%d",&size);
  if(size > MAX)
  {
    printf("Size Exceeds Buffer\n");
    return FAILURE;
  }

  retval = read(fd, rdbuff, size);
  if(retval < 0)
  {
    perror("Read Failed : ");
    return FAILURE;
  }
  printf("File Read Success\n");

  for(ii = 0; ii < size; ii++)
    printf("%d ",rdbuff[ii]);
  printf("\r\n");

  return SUCCESS;
}

int writeFile(int argc,char *argv[])
{
  int retval,ii,size = 0;

  if(argc != 2)
  {
    printf("Usage <Cmd> <Size>\n");
    return FAILURE;
  }
  sscanf(argv[1],"%d",&size);
  if(size > MAX)
  {
    printf("Size Exceeds Buffer\n");
    return FAILURE;
  }

  for(ii = 0; ii < size; ii++)
    wrbuff[ii] = ii;

  retval = write(fd, wrbuff, size);
  if(retval < 0)
  {
    perror("Write Failed : ");
    return FAILURE;
  }
  printf("File Write Success\n");

  return SUCCESS;
}

int closeFile(int argc,char *argv[])
{
  if(0 > close(fd))
  {
    perror("Close Failed : ");
    return FAILURE;
  }
  printf("File Close Success\n");
  return SUCCESS;
}

int seekFile(int argc,char *argv[])
{
  uint32_t retval, pos = 0;

  if(argc != 2)
  {
    printf("Usage <Cmd> <Position>\n");
    return FAILURE;
  }
  sscanf(argv[1],"%u",&pos);
  if(pos > MAX)
  {
    printf("Position Exceeds Buffer Size\n");
    return FAILURE;
  }

  retval = lseek(fd, pos, SEEK_SET);
  if(-1 == retval)
  {
    perror("Lseek Failed : ");
    return FAILURE;
  }
  printf("Current Position %d\n",retval);
  return SUCCESS;
}

int quitApp(int argc,char *argv[])
{
  quitFlag = 1;
  return 0;
}

int dispHlp(int argc,char *argv[])
{
  int ii;
  for(ii = 0; ii < (sizeof(commandTable)/sizeof(cmdFun_t)); ii++)
    printf("%s\t : %s\n",commandTable[ii].cmd,commandTable[ii].hlpStr);
  printf("\n");
  return 0;
}
