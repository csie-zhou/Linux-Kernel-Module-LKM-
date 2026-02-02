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
