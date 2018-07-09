#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>

#define SUCCESS  0

#define DEVICE_NAME "buffDevice"

/* PAGE_SIZE is the default Macro */
#define NO_OF_PAGES 10
#define MAX_LENGTH (NO_OF_PAGES * PAGE_SIZE)

static int inuse = 0;
char *ptr = NULL;

static int device_open(struct inode *inode, struct file *file)
{
  if(inuse)
  {
    pr_err("Device Busy %s\r\n",DEVICE_NAME);
    return -EBUSY;
  }
  inuse = 1;
  pr_info("Open Operation Invoked\r\n");
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
  inuse = 0;
  pr_info("Release Operation Invoked\r\n");
  return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
  int maxbytes    = 0;  //Number of bytes from ppos to MAX_LENGTH
  int bytes_to_do = 0;  //Number of bytes to read
  int nbytes      = 0;  //Number of bytes actually read

  maxbytes = MAX_LENGTH - *ppos;

  if(maxbytes > size)
    bytes_to_do = size;
  else if(maxbytes == size)
    bytes_to_do = maxbytes;
  
  if(0 == bytes_to_do)
  {
    pr_err("Reached End of Device\r\n");
    return -ENOSPC;
  }
  pr_info("#1 %lld\r\n",*ppos);
  nbytes = bytes_to_do - copy_to_user(buf, ptr + *ppos, bytes_to_do);
  *ppos += nbytes;

  pr_info("Read Operation Invoked\r\n");
  return nbytes;
}

static ssize_t device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
  int maxbytes    = 0; // Maximum number of bytes that can be written
  int bytes_to_do = 0; // Number of bytes to write
  int nbytes      = 0; // Number of bytes written

  maxbytes = MAX_LENGTH - *ppos;
 
  if( maxbytes > size )
    bytes_to_do = size;
  else
    bytes_to_do = maxbytes;

  if(0 == bytes_to_do)
  {
    printk("Reached end of device\r\n");
    return -ENOSPC;
  }
  pr_info("#2 %lld\r\n",*ppos);
  nbytes = bytes_to_do - copy_from_user(ptr + *ppos, buf, bytes_to_do);
  *ppos += nbytes;
  
  pr_info("Write Operation Invoked\r\n");
  return nbytes;
}

static loff_t device_lseek(struct file *filp, loff_t offset, int orig)
{
  loff_t new_pos = 0;

  switch(orig)
  {
    /* SEEK_SET */
    case 0:
      new_pos = offset;
      break;

    /* SEEK_CUR */
    case 1:
      new_pos = filp->f_pos + offset;
      break;

    /* SEEK_END */
    case 2:
      new_pos = MAX_LENGTH - offset;
      break;
  }

  if(new_pos > MAX_LENGTH)
    new_pos = MAX_LENGTH;
  
  if(new_pos < 0)
    new_pos = 0;
  
  filp->f_pos = new_pos;
  pr_info("Seek Operation Invoked\r\n");
  return new_pos;
}

static struct file_operations device_fops = {
  .owner   = THIS_MODULE,
  .open    = device_open,
  .release = device_release,
  .read    = device_read,
  .write   = device_write,
  .llseek  = device_lseek,
};

static struct miscdevice device_misc = {
  .minor = MISC_DYNAMIC_MINOR,
  .name  = DEVICE_NAME,
  .fops  = &device_fops
};

/* Register Driver with Misc Interface */
static int __init device_init(void)
{
  int retval;

  /* Register a Miscellaneous Device */
  retval = misc_register(&device_misc);
  if(retval < 0)
  {  
    pr_err("Device Registration Failed with Minor Number %d\r\n",device_misc.minor);
    return retval;
  }
  pr_info("Device Registered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);

  /* Allocate Slab */
  ptr = kmalloc(MAX_LENGTH, GFP_KERNEL);
  if(NULL == ptr)
  {
    pr_err("Slab Allocation Failed\r\n");
    return -ENOMEM;
  }
  return SUCCESS;
}

/* Unregister Driver */
static void __exit device_exit(void)
{  
  /* Free the allocated slab */
  kfree(ptr);

  /* Unregister the Miscellaneous Device */
  misc_deregister(&device_misc);

  pr_info("Device Unregistered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);
}

/* Register the names of custom entry and exit routines */
module_init(device_init);
module_exit(device_exit);

/* Comments which are retained in code */
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_DESCRIPTION("Generic Char Driver with Virtual Buffer as a Device");
MODULE_LICENSE("GPL");

