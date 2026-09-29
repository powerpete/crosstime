#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/device.h>

#define DEVICE_NAME "crosstime"

static int            majorNumber;
static struct class*  crosstimeClass  = NULL;
static struct device* crosstimeDevice = NULL;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);

static const struct file_operations fops =
{
   .owner   = THIS_MODULE,
   .open    = dev_open,
   .read    = dev_read,
   .release = dev_release,
};

static char *crosstime_devnode(const struct device *dev, umode_t *mode)
{
   if (mode)
      *mode = 0444;
   return NULL;
}

static int __init crosstime_init(void)
{
   printk(KERN_INFO DEVICE_NAME " Initializing the Kernel module\n");

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber < 0) {
      printk(KERN_ALERT DEVICE_NAME " failed to register a major number\n");
      return majorNumber;
   }

   crosstimeClass = class_create("crosstime_class");
   if (IS_ERR(crosstimeClass)) {
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(crosstimeClass);
   }
   crosstimeClass->devnode = crosstime_devnode;
   crosstimeDevice = device_create(crosstimeClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(crosstimeDevice)) {
      class_destroy(crosstimeClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(crosstimeDevice);
   }

   return 0;
}

static void __exit crosstime_exit(void)
{
   device_destroy(crosstimeClass, MKDEV(majorNumber, 0));
   class_destroy(crosstimeClass);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk(KERN_INFO DEVICE_NAME " Module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO DEVICE_NAME " Device has been opened\n");
   return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO DEVICE_NAME " Device successfully closed\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
   ktime_t parts[3];

   if (len < 2*sizeof(ktime_t))
      return -EINVAL;

   parts[0] = ktime_get();
   parts[1] = ktime_get_real();
   parts[2] = ktime_get();
   
   parts[2] = parts[0] + (parts[2] - parts[0]) / 2;
    
   if (copy_to_user(buffer, &parts[1], 2*sizeof(ktime_t)) != 0)
      return -EFAULT;

   return 2*sizeof(ktime_t);
}

module_init(crosstime_init);
module_exit(crosstime_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("create a simple device for crosstime reading");
MODULE_VERSION("0.1");
