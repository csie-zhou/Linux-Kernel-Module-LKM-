# Create Linux Env (If you haven't done)
My Device: Macbook M2 Pro  

Virtual Machine: **UTM**  
- It’s free, open-source, and designed specifically for macOS. It actually uses QEMU under the hood but gives you a nice interface.  
- (Disadvantages: If you crash the kernel, you usually have to restart the whole VM. On Apple Silicon, most VMs use "Virtualization," meaning you are restricted to ARM-based Linux distros.)  

## 1. Download [UTM](https://mac.getutm.app/) App
## 2. Download the correct ISO
Since I am on an M-series Mac, I must use an `arm64` (AArch64) version of Linux for performance.
- Used: [Ubuntu Server 24.04 for ARM](https://ubuntu.com/download/server/arm)
- *Note: Use the "Server" version to save resources, you can add a GUI later if you really need it.*
## 3. Create the VM in UTM
1. Open UTM and click **Create a New Virtual Machine.**
2. Select **Virtualize** (do not select "Emulate" or it will be 10x slower).
3. Select **Linux.**
4. Under **Boot ISO Image**, browse and select the Ubuntu ISO you downloaded.
5. **Hardware:** Allocate at least 4GB of RAM and 4 CPU cores.
6. **Storage:** 20GB-30GB is enough for kernel development.
7. **Shared Directory**: (Optional) Select a folder on your Mac (e.g., `~/Desktop/Code`) so you can write code on your Mac and compile it in Linux.

## 4. The Ubuntu Installation Process
1. Start the VM. It will boot into the Ubuntu installer.
<img width="600" height="450" alt="截圖 2026-02-02 下午6 40 41" src="https://github.com/user-attachments/assets/acc50a00-d5c3-4a3e-bdc6-460f7c8490fe" />

2. Select **Try or Install Ubuntu Server**
3. **Language:** default "English (US)".
4. **Installation:** Choose the default **"Ubuntu Server"** (not the minimized version).
5. **Storage:** Use the default **"Use an entire disk.", "Set up this disk as an LVM group"**
6. **Profile:** Create your username and password.
7. **SSH:** Select **"Install OpenSSH Server"** (highly recommended so you can use your Mac's Terminal to log in), don't need to **"Import SSH key"**.
8. (Others not mentioned leave default or blank.)
9. Once finished, select **Reboot.**
10. **Crucial Step:** When the VM window closes, go back to the UTM main screen, find the "CD/DVD" dropdown for that VM, and click **Clear.** Then start it again.

### Trouble Shooting in Installation
- If you find this after you press "Reboot". The reason you are stuck in a loop is that the installer is still the "first priority" in your boot sequence.
<img width="453" height="71" alt="截圖 2026-02-02 晚上8 10 05" src="https://github.com/user-attachments/assets/89d190bf-cd89-4170-8892-a3bd5276834a" />

Steps to fix:
1. Force Shutdown the VM: click the **Power icon** in the top-left corner of the UTM window to force the VM to turn off.
2. Eject the "CD" (Clear the Drive):
  - Go to the main UTM library window where your VM is listed.
  - Look at the Drive or **CD/DVD** section at the bottom right.
  - Find the row showing the `ubuntu-24.04.3...iso` file.
  - Click the dropdown menu next to it and select **Clear.**
  - The status should now say "Empty".
3. Press "Play", it will now boot into the version of Ubuntu where you created.
4. Login (eg. `linux-dev login:`), type your username and password.

## 5. Mount Linux VM to Local
In my recently setup, I selected `~/Documents/Work/Project/linux_kernel_moudle/coding` (yours may differ) as my shared folder. To see these files inside Linux, we should mount them:  
1. In UTM interface, find **Settings** and click. Find **Sharing** tab, ensure the entry is pointing to the folder (eg. `.../coding` will show as `coding`).
2. Ensure "Directory Share Mode" is set to **virtiofs**.
3. Check the **QEMU** tablet in the left hand side, if exists `-device vhost-user-fs-pci,tag=share` like.
  - `tag`: The tag stands for your local folder, default is `share`.
  - Notice the portocol in the string, `vhost-user-fs-pci` or `virtio-9p-pci`. Differ in later setup codes.  
4. Login to VM
5. Install Packages:
```
sudo apt update
sudo apt install -y virtiofsd attr
```
6. Create a mount piont:
```
mkdir ~/project
```
7. Mount the folder:
```Bash
# If your portocol is `vhost-user-fs-pci` or something else:
sudo mount -t virtiofs share ~/project

# If your portocol is `virtio-9p-pci`, which is an older version:
sudo mount -t 9p -o trans=virtio share ~/project -oversion=9p2000.L
```

You will now see codes in the Linux VM checking by `ls ~/project`.

## 6. Remote SSH on VS Code
1. Open VS Code, download **"Remote-SSH"** Extension.
2. `Cmd + Shift + P`, select `Remote-SSH: Connect to Host...`.
3. Enter connection information (eg. `danny@192.168.64.2`)
4. Choose `/User/zhou/.ssh/config` (something alike) as SSH Configuration
5. After connected, open folder in VS code, choose `/home/danny/project` (choose yours)
6. Install packages
```
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

### Trouble Shooting
If you successfully mounted the local folder to Linux VM, but meeting problems that you cannot get the permission in VS Code. This issue can be that VS Code is logged in Linux, but does not get the permission from Linux Server.  

Try this (it works for me):  
1. Find your user ID and group ID (default=`1000`):
```
id
```
2. Edit the mount configuration:
```
sudo nano /etc/fstab

# Add or modify the line to include `uid` and `gid`:
share /home/danny/project 9p trans=virtio,version=9p2000.L,rw,uid=1000,gid=1000,_netdev 0 0
```
`Ctrl + O` + `Enter` + `Ctrl + W` to exit nano.  
3. Remount:
```
sudo umount /home/danny/project
sudo mount -a
```
4. SSH into your VM and change the ownership of the shared folder:
```
sudo chown -R danny:danny /home/danny/project
```
5. Reboot:
```
sudo reboot
```

# Hello World
Test all our work sticks together (Linux VM + Remote SSH in VS Code). Now, we are able to modify files in VS Code, the files should be synchronous to local folder and Linux VM.  
## 1. Create hello.c
Create a `hello.c` file in VS Code, copy paste this:
```C
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
```
## 2. Create Makefile
Touch a Makefile, copy paste this:
```
obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
## 3. Make and Test
1. Compile module: `make`
2. Load module to kernel: `sudo insmod hello.ko`
3. Remove module from kernel: `sudo rmmod hello`
4. Check message in kernel: `sudo dmesg | grep` or `sudo dmesg | tail -5`
5. Expected:
```
[  +9.245033] Hello World! Your LKM environment is fully operational.
[Feb 2 19:49] Goodbye World! Module successfully removed.
```

### Pro Tip
Open two terminals to watch kernel message in real time.  
**Terminal 1:**
```
sudo dmesg -wH
```
**Terminal 2:**
```
sudo insmod hello.ko
sudo rmmod hello
sudo insmod hello.ko
sudo rmmod hello
```

### Trouble Shooting
If you have includePath error in `hello.c`, follow the steps:  
1. Check your architechture: `uname -m` (eg. aarch64).
2. In VS Code, press `Ctrl+Shift+P`, choose "C/C++: Edit Configurations (JSON)"
3. Modify "includePath" ("defines" if needed):
```
 {
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/src/linux-headers-6.8.0-94-generic/include",
                "/usr/src/linux-headers-6.8.0-94-generic/arch/arm64/include",
                "/usr/src/linux-headers-6.8.0-94-generic/arch/arm64/include/generated",
                "/usr/src/linux-headers-6.8.0-94/include",
                "/usr/src/linux-headers-6.8.0-94/arch/arm64/include",
                "/usr/include"
            ],
            "defines": [
                "__KERNEL__",
                "MODULE"
            ],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-arm64"
        }
    ],
    "version": 4
}
```
