# Chapter 1. The Skeleton and File Operations
In the kernel, we don't have `main()` function. Instead, we have **Initialization** and **Exit** functions. 
We also need to define **File Operations** (`fops`) so the kernel knows what to do when a user tries to 
`open`, `read`, or `write` (API in User Space) to your driver.

## Key Tasks
1. Build a **Character Device Driver.**

## Build a Character Device Driver
This is a special type of driver that allows a User Space program to communicate with the Kernel as if it were reading or
 writing to a file. Every device in Linux has a **Device Number**, consists of **Major Number** and **Minor Number**.
   - Major Number: Often represents a category of devices.
   - Minor Number: Number to show the specific Deivce.  

You can type `cat /proc/devices` in terminal, and it will show a list of devices with "Major Number" and "Device Name".  

When the system identifies a device driver, the driver will register a **Deivce Number** from **Kernel**, and create a **Device File**
 in `/dev`.
### 1. The Kernel Module Template
Create `my_bridge.c` with this:
```C
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
```
Explanation:
1. `static`: It's is essential in Kerenl Space.
  - **Namespace Protection:** The kernel is a giant program. If you name a function `open()` without `static`, and another driver 
  also has a function named `open()`, the kerenl wil crash (symbol collision).
  - **Encapsulation:** By using `static`, you can ensure those are visible only in your `.c` file. No other file can access them.
2. `struct` (`struct inode *inodep`): In Linux Kerenl C, you must explicitly say `struct` every time you use it unless someone created a `typedef`. 
But the kernel developers avoid `typedef` for structures to remind themselves exactly waht kind of data they are handling.
3. `inode` vs `file`: 
  - `inode`: Represents the file as it exist on the disk. It doesn't change regarless of how many people open it.
  - `file`: Represents an **instance** of an open file. If two different programs open your driver, there is one `inode` 
  but two `file` structure (each with its own cursor or state).
4. `fops` sturcture: This is called **Designed Initializers**
    ```C
    static struct file_operations fops = { .open = dev_open };
    ```
  - In stead of remembering the order of 30+ functions in the structure (`file_operation`), this tells the compiler to find the 
member named `open` and point to my `dv_open`.

5. `__init` in `static int __init bridge_init`: It's a **Compiler Marco**.
  - **Memory Optimization**: When the kernel finishes running the `bridge_init` function, it knows that function will **never**
   be called again. The `__init` tag tells the kernel: "After you run this, you can delete this code from RAM to save space."
  - **Double Underscores(`__`)**: In C, double underscores usally signal **"Internal Kernel Logic"** or **"Low-Level Compiler Attribute"**.
   It warns the programmer: "This function is for kernel/compiler."
### 2. Makefile
Create a `Makefile`:
```
obj-m += my_bridge.o

all:
 make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
### 3. Compile and Test
1. Run `make`
2. Load the module: `sudo insmod my_bridge.ko`
3. Check if loaded: `sudo dmesg | tail`
4. Remove it: `sudo rmmod my_bridge`
5. Expectations: (`sudo dmesg | tail` again)
 ```
 [ 9454.551509] Bridge: registered with major number 234
 [ 9461.499324] Bridge: Goodbye from the Kernel!
 ```
