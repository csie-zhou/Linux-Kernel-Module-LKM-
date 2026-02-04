# Chapter 2. The Memory Bridge
After Chapter 1, we can successfully build and load the module to kernel. In this Chapter, we are going to implement **Read** and **Write**.  

In the kernel, we can not use `memcpy`. If a User Space program gives you a pointer, you cannot simply "dereference" it. 
If the user gives you a bad address, the whole system wiil crash.  

## Key Tasks
1. Implement **Read** and **Write**.

### 1. Implement Read
#### copy_to_user
To let a user **read** data from our driver, we must use the function: `copy_to_user(to, from, n)`, where:
  - `to`: Where the data is going (the user's buffer).
  - `from`: Where the data is coming from (our kernel variable).
  - `n`: How many bytes to move.


Add these in `my_bridge.c`:
```C
static char message[256] = "Hello from the Kernel!";

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
```

Explanations:
1. `ssize_t`: In the kernel, we use `ssize_t` for return values of read/write operations for two reasons:
  - **Signed Size**: The `s` stands for signed. `size_t` is unsigned. Return value has to be a positve number of bytes on success,
   or **negative** error code (like `-EFAULT` or `EINVAL`) on failure.
  - **Architecture Portability**: `ssize_t` is guaranteed to be the same size as a pointer on your specific architecture. On our 
   64-bit Linux VM, it's 64-bit, whereas standard `int` is 32-bit. Using `int` could cause overflow if we ever tried to read more than 2GB data at once. 
2. `*offset`: Acts like a bookmark. Starting from 0. When you `read` 10 bytes, the kernel moves the offset to 10. If the user
 calls `read` again, the kernel asks the data **starting from 10.**
3. If we don't check the offset (by `return strlen(message)`), the user will be stuck in an infinite loop. They will read "Hello"
 again and again.
4. We return `0` when `*offset > 0` to tell the user, we reached the end of the message.

### 2. Implement Write
#### copy_from_user
To let a user **write** data back to kernel, we must use the function: `copy_from_user(to, from, n)`, where:
  - `to`: Our kernel variable `message`.
  - `from`: The user's `buffer`.
  - `n`: How many bytes to move.

Add these in `my_bridge.c`:
```C
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    size_t datalen = len > 255 ? 255 : len;
    
    // Reset message buffer to zeros before writing new data
    memset(message, 0, sizeof(message));

    // 1. TODO: Use the bridge function to take data FROM the user and put it into 'message'
    // Warning: Don't copy more than the size of 'message' (256 bytes)!
    int uncopied = copy_from_user(message, buffer, datalen);

    if (uncopied == 0) {
        message[255] = '\0';
        printk(KERN_INFO "Bridge: Accepted %zu characters from user\n", datalen);
        return datalen;    
    } else {
        return -EFAULT;
    }    
}
```

Explanations:
1. You can see the line `len > 255` (not 256), cause in C string, you have to leave the last slot for terminator `\0`.
2. `memset`, `memcpy`, `strlen`, `printk` are built-in libraries for kernel (in `lib/string.c`). But `<string.h>`, `<stdio.h>`
 are unvailable headers.
3. We `memset` the kernel space to `0`, which is same as the terminator `\0`. Note this is not same as ASCII character `'0'`. 
 So we don't need to assign the terminator after `copy_from`user`, only need to ensure the last character ends the string.

### 3. Test
1. Update `fops` to include new functions:
    ```C
    static struct file_operations fops = {
        .open = dev_open,
        .read = dev_read,
        .write = dev_write,
        .release = dev_release,
    };
    ```
2. Add these at the definition in `my_bridge.c`:
    ```
    // ... dev_release ... ;
    static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset);
    static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);
    ```
3. `make` and `sudo insmod my_bridge.ko`.
4. We now loaded the **Kernel Module**, but the **Device Node** (the file in `/dev`) hasn't been created yet. In Linux, loading
 a module doesn't automatically create a file in `/dev`.
    - **Find the Major Number:** `dmesg | grep "Bridge: Registered"`, you will see a line like `Bridge: Registered with major number 234`
     (the number may be different)
    - **Manually Create the Node:**  `sudo mknod /dev/my_bridge c 234 0` (replace `234` with your number). `c` stands for 
     **Character** device; `234` is the **Major Number**; `0` is the **Minor Number**.
5. Get permission: `sudo chmod 666 /dev/my_bridge`
6. **Test Writing**: `echo "Hi Kernel" > /dev/my_bridge`
7. **Test Reading**: `cat /dev/my_bridge`
8. Remove Driver: `sudo rmmod my_bridge`

9. Expextation:  
    In terminal/user space (after `cat /dev/my_bridge`):
    ```
    Hi Kernel
    ```
    In `sudo dmesg | tail`:
    ```
    [17009.891570] Bridge: Registered with major number 234
    [17329.913189] Bridge: Device has been opened
    [17329.913205] Bridge: Accepted 10 characters from user
    [17329.913208] Bridge: Device successfully closed
    [17344.811088] Bridge: Device has been opened
    [17344.811138] Bridge: Sent 10 characters to the user
    [17344.811189] Bridge: Device successfully closed
    [17360.678736] Bridge: Goodbye from the Kernel!
    ```
#### Explanation
1. `insmod` vs `mknod`:
   - `insmod` installs the **Driver** (the software) to kernel. Kernel knows it exists, but no plug for user to access it. 
   - `mknod` installs the **Interface** to `/dev`. It creates a file that says: *"Every access to this file is actually talking to Driver #234."*
   - We will learn how to create the node automatically in the next chapter.
2. `sudo chmod 666 /dev/my_bridge`: **Change mode** of the file `/dev/my_bridge`. Linux has threes sets of permission in every file: 
**Owner**, **Group** and **Others**. The number `6` stands for **Read(4) + Write(2) = 6**.  
*(If you didin't do it, only root can talk to driver. Once you allowed all user, you don't need to `sudo` to access.)*
3. The behavior of `dev_write`: Once the driver is insert in the kernel, and so as the interface, now we can 
start to interact with kernel using the interface we wrote: `struct file_operation fops`. Think of the structure as Lookup Table.

    When we run `echo "Hi" > /dev/my_bridge`:
     - **User Space:** `echo` opens the file and calls standard Linux System Call `write()`.
     - **The Kernel:** Recieves `write()` request. It looks at `/dev/my_bridge` and sees it has a **Major Number 234**.
     - **The Lookup:** The Kernel checks its list of drivers, finds the 234, which is our module.
     - **Switchboard**:  The Kernel looks at our `fops`, finds out we modify the `write()`: 
         ```
         .write = dev_write,
         ```
     - **Execution:** The Kernel executes our `dev_write` function.
4. The behavior of `dev_read`: By `cat /dev/my_bridge`. Inside `dev_read`, we called `copy_to_user(..., message, ...)`.
5. Overall: We overworte with `echo` the `"Hi Kernel"` first, then use `cat` to read it. So the message is overwritten, 
the characters `10` is the length of the "Hi Kernel" **including `\0`**. If you want to show the original message, run `cat` before `echo`.
