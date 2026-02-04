# Linux-Kernel-Module-LKM

- Design and implement a **Linux Kernel Module** in **C** that manages a virtual character device to facilitate communication **between User Space and Kernel Space.**
- Manage **hardware-software synchronization** using **Spinlocks** and **Mutexes** to prevent race conditions during concurrent I/O operations.
- Implement `mmap` (Memory-Mapped I/O) support to enable zero-copy data transfers, significantly reducing CPU overhead for high-throughput applications.

## Keywords
### 1. Module (`.ko` file)  
- What: A piece of code that can be "loaded" or "unloaded" from the kernel at any time.
- Where: `module_init` and `module_exit` functions.
- Concept: Module is a **Container.** It usually contains a driver, but it could also contain a filesystem, a firewall rule, or network protocol.  
    - Command: `sudo insmod my_bridge.ko` (Activating the module).
### 2. Dirver (`fops`)
- What: The **Job Description**. The software logic that knows how to talk to a specific hardware. It defines "What happens when I read/write?".
- Where: `struct file_operations fops`.
- Concept: Dirver is **Logic**. It translates generic request into specific actions.
    - Connection: The "Major Number" identifies which Driver to use.

### 3. Class(`/sys/class`)
- What: A **category**. The Class groups devices by **what the do**, not how they work.
- Where: We use `bridge_class = class_create(...)` for **Automation**.
- Concept: When we ran `device_create`, we were telling the Kernel: *"Take this specific Device under this Class"*. The system mansger `udev` watches these classes. As soon as it sees a new device appear under a Class, it automatically runs to the "Service Counter" (`/dev/`) and creates the **Node** for you.
    - A mouse, a trackpad, and a drawing tablet are all very different hardware, but they all belong to the **Input Class.**
    - Your speaker and your headphones are different, but they belong to the **Sound Class.**
    
### 4. Device(`struct device`)
- What: The internal object the represents the actual "thing" being managed.
    - If you have 4 USB drives plugged in, you have **1 Driver** (USB Storage Driver) but **4 Devices**.
- Where: The `bridge_device` pointer created by `device_create`.
- Concept: Device is the **Instance**.
    - Connection: The "Minor Number" identifies which specific Device instance you are talking to.

### 5. Node(`/dev/my_bridge`)
- What: Like a service window. A file sitting on the hard drive (in `/dev`) that allows User Space program (like `echo` or `cat`) to "talk" to Kernel. It is the **Interface**.
- Where: Not written in the code. We use `device_create` to tell the kernel to make the node for our.
- Concept: Node is **Interface**. It has no logic itself; it just forwards our requests to the Driver.

### Interaction
- You **insmod** the **Module.**
- The Module registers the **Driver.**
- The Driver manages the **Device.**
- The User talks to the **Node.**

## Overview
Define these terms as a Restaurant:  
| Term  |      Analogy                           |    Code                |
|:------| :-------                               |   :------              |
|Module |The **Employee**                        |`insmod`/`rmmod`        |
|Driver |The **Skill Set** (Recipe)              |`struct file_operations`|
|Class  |The **Department** (Category)           |`class_create`          |
|Device |The **Workstation** (Specifc Stove)     |`device_create`         |
|Node   |The **Service Window** (User Interface) |`/dev/my_bridge`        |

## The Flow
When you run `echo "Hi" > /dev/my_bridge`:
1. **User Space:** Touches the **Node** (/dev/my_bridge).
2. **Kernel Switchboard:** Looks at the Node's numbers (Major 234).
3. **The Driver:** The kernel wakes up the Driver registered to 234.
4. **The Module:** The Driver code lives inside the Module memory.
5. **The Action:** The Driver executes dev_write to change the data.
