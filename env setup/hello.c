#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// This function runs when the module is loaded
static int __init hello_world_init(void)
{
    printk(KERN_INFO "Hello World! Your LKM environment is fully operational.\n");
    return 0; // A non-zero return means the module failed to load
}

// This function runs when the module is removed
static void __exit hello_world_exit(void)
{
    printk(KERN_INFO "Goodbye World! Module successfully removed.\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danny Chou");
MODULE_DESCRIPTION("A simple test module for a new dev environment.");
