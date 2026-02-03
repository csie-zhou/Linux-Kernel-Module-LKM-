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
Create 
```C
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

