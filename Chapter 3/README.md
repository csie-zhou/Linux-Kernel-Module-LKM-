# Chapter 3. Automation
In this chapter, we will use `udev` (User Device Management) to create a node automatically. 

## Key Tasks
1. add `class_create` and `device_create` to the `__init` function.
2. add `device_destroy` and `class_destroy` to the exit function.

### 1. Add new Header
Add these include files at the top of `my_bridge.c`:
```C
#include <linux/device.h>   // Required for the device model
#include <linux/err.h>      // Required for error handling (IS_ERR)

static struct class *bridge_class = NULL;   // The device class
static struct device *bridge_device = NULL; // The device object
```
### 2. Modify __init
Modify the `__init` function to this:
```C
static int __init my_bridge_init(void)
{
    // 1. Register the major number
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0)
        return majorNumber;

    // 2. Register the device class
    bridge_class = class_create(DEVICE_NAME); // Note: kernel 6.4 uses class_create(name)
    if (IS_ERR(bridge_class))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return PTR_ERR(bridge_class);
    }

    // 3. Create the device node automatically
    // device_create needs: class, a parent (NULL), device ID, extra data (NULL), device name
    bridge_device = device_create(bridge_class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(bridge_device))
    {
        class_destroy(bridge_class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return PTR_ERR(bridge_device);
    }

    printk(KERN_INFO "Bridge: device class created correctly\n");
    return 0;
}
```
- When we create the device, we ask **Class** to register a specific **ID** (`MKDEV(majorNumber, 0)`).

### 3. Modify __exit
You can try doing it on your own first. Similar as above, here's a correct version:
```C
static void __exit my_bridge_exit(void)
{
    // TODO
    // 1. Remove the device node (/dev/my_bridge)
    // Note: We use MKDEV to reconstruct the ID we use in init
    device_destroy(bridge_class, MKDEV(majorNumber, 0));

    // 2. Remove the class (/sys/class/my_bridge)
    class_destroy(bridge_class);

    // 3. Unregister the major number
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "Bridge: Goodbye from the Kernel!\n");
}
```
- To destroy the device, we must tell the **Class** exactly which **ID** to remove. The pointer `bridge_device` isn't needed here.
### 4. Compile
1. `make`
2. Load: `sudo insmod my_bridge.ko`
3. Check: Run `ls -l /dev/my_bridge`, if you see the file exists without using `mknod`, then you have successfully implemented **Linux Device Model** automation!
