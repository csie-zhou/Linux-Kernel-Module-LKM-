#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>      // Require for file operations
#include <linux/uaccess.h> // Required for copy_to_user / copy_from_user

#define DEVICE_NAME "my_bridge"
static char message[256] = "Hello from the Kernel!";

// 1. Define the function signatures for opening and closing the device
static int dev_open(struct inode *inodep, struct file *filep);
static int dev_release(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);

// 2. File Operations Structure
// This maps system calls (like 'open') to your function below.
static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
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
    printk(KERN_INFO "Bridge: Registered with major number %d\n", majorNumber);
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

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;

    // We only want to send the message once (check if offset is 0)
    if(*offset > 0) return 0;

    // TODO: Use the bridge function to send 'message' to the 'buffer'
    // It returns the number of bytes that COULD NOT be copied (0 is success)
    error_count = copy_to_user(buffer, message, strlen(message));

    if (error_count == 0) {
        printk(KERN_INFO "Bridge: Sent %zu characters to the user\n", strlen(message)); // %zu: unsigned size_t
        *offset += strlen(message);
        return strlen(message);
    } else {
        printk(KERN_INFO "Bridge: Failed to send %d characters\n", error_count);
        return -EFAULT; // Return a "Bad Address" error
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    size_t datalen = len > 255 ? 255 : len;
    
    // Reset message buffer to zeros before writing new data
    memset(message, 0, sizeof(message));

    // 1. TODO: Use the bridge function to take data FROM the user and put it into 'message'
    // Warning: Don't copy more than the size of 'message' (256 bytes)!
    int uncopied = copy_from_user(message, buffer, datalen);

    if (uncopied == 0) {
        message[255] = '\0';
        printk(KERN_INFO "Bridge: Accepted %zu characters from user\n", len);
        return datalen;    
    } else {
        return -EFAULT;
    }    
}

module_init(my_bridge_init);
module_exit(my_bridge_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danny Chou");
MODULE_DESCRIPTION("A simple Linux char driver for system software design.");