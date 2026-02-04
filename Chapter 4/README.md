# Chapter 4. Race Condition
Consider of a scenario, two terminals `echo` to the same driver:
  - User A: `echo "User A here" > /dev/my_bridge`
  - User B: `echo "User B here" > /dev/my_bridge`  

If they hit Enter at the exact time:
1. **User A** passes the check `if (uncopied == 0)`.
2. **User B** interrupts and overwrites the `message` buffer with "User B here".
3. **User A** finishes and `printk` "Accepted User A Data ...", but the memory actually contains "User B here".
4. **Data Corruption**.

## Key Tasks
1. Implement **Mutex Lock** in kernel.

### 1. Declare the Lock
Add this marco to create a lock and sets it to "unlocked" state automatically.
```C
#include <linux/mutex.h> // Required for the mutex functionality

static DEFINE_MUTEX(bridge_mutex); // A global lock for this driver
```
### 2. Lock the Critical Section (`dev_write`)
We must wrap the code that touches the shared memory (`message`) with lock and unlock calls. 
If `copy_from_user` fails, we must unlock before returning error, otherwise the driver stays locked forever (Deadlock)!  

Copy paste the newest `dev_write`:
```C
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    size_t datalen = len > 255 ? 255 : len;

    // 1. Try to take the lock. If interrupted by a signal, return error.
    if (mutex_lock_interruptible(&bridge_mutex))
    {
        return -ERESTARTSYS;
    }

    /* --- Critical Section Starts --- */

    // Reset message buffer to zeros before writing new data
    memset(message, 0, sizeof(message));

    // 1. TODO: Use the bridge function to take data FROM the user and put it into 'message'
    // Warning: Don't copy more than the size of 'message' (256 bytes)!
    int uncopied = copy_from_user(message, buffer, datalen);

    if (uncopied != 0)
    {
        return -EFAULT;
    }

    message[255] = '\0';
    printk(KERN_INFO "Bridge: Accepted %zu characters from user\n", datalen);

    /* --- Critical Section Ends --- */
    // 2. Give back the lock
    mutex_unlock(&bridge_mutex);
    return datalen;
}
```

### 3. Lock the Critical Section (`dev_read`)
Race Condition can also happen in read, if a user starts reading while the other user wirtes.  

Copy paste the newest `dev_read`. Look closely at the `goto` statement - this is the standard kernel pattern for clean error handling.
```C
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;
    ssize_t ret_val = 0;               // Use a variable for success / failure return value.
    ssize_t msg_len = strlen(message); // New a variable to lock this also

    if (mutex_lock_interruptible(&bridge_mutex))
    {
        return -ERESTARTSYS;
    }
    /* --- Critcal Section Starts --- */

    // We only want to send the message once (check if offset is 0)
    if (*offset > 0)
    {
        ret_val = 0; // End of file
        goto out;    // Jump to exit
    }

    // 2. Perform the Copy
    // It returns the number of bytes that COULD NOT be copied (0 is success)
    error_count = copy_to_user(buffer, message, msg_len);

    if (error_count == 0)
    {
        printk(KERN_INFO "Bridge: Sent %zu characters to the user\n", msg_len); // %zu: unsigned size_t
        *offset += msg_len;
        ret_val = msg_len;
    }
    else
    {
        printk(KERN_INFO "Bridge: Failed to send %d characters\n", error_count);
        ret_val = -EFAULT; // Return a "Bad Address" error
    }
out:
    // This is the only way out of this function (unlock -> return), except the lock error at the top.
    mutex_unlock(&bridge_mutex);
    return ret_val;
}
```
  
