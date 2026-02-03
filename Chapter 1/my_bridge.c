#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>      // Require for file operations
#include <linux/uaccess.h> // Required for copy_to_user / copy_from_user

#define DEVICE_NAME "my_bridge"

// 1. Define the function signatures for opening and closing the device
static int dev_open(struct inode *inodep, struct file *filep);
static int dev_release(struct inode *inodep, struct file *filep);

// 2. File Operations Structure
// This maps system calls (like 'open') to your function below.
static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    // We will add .read and .write in the next chapter
};

static int majorNumber;

// Initialization Function
static int __init my_bridge_init(void)
{
    // 3. TODO: Register the character device.
    // Hint: Look up 'register_chrdev'. It needs a major number (0 for auto, dynamic allocation), a name, and our fops.}
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    if (majorNumber < 0)
    {
        printk(KERN_ALERT "Bridge failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "Bridge: registered with major number %d\n", majorNumber);
    return 0;
}

// Exit Function
static void __exit my_bridge_exit(void)
{
    // 4. TODO: Unregister the device to prevent kernel memory leaks.
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "Bridge: Goodbye from the Kernel!\n");
}

// Function implementation
static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Bridge: Device has been opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Bridge: Device successfully closed\n");
    return 0;
}

module_init(my_bridge_init);
module_exit(my_bridge_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danny Chou");
MODULE_DESCRIPTION("A simple Linux char driver for system software design.");