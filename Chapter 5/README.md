# Chapter 5. Implementing mmap
In this chapter, we are adding `mmap` operation to the driver.  

1. Allocate a full "Page" of memory (4096 bytes) in the kernel.
2. When the user calls `mmap()`, we map that physical page into their process.
3. They can read/write to it like a normal array, by pressing `read()` and `write()` system calls entirely.
## Key Tasks

### 1. Add Memory Management Headers
```C
#include <linux/mm.h>      // Memory management
#include <linux/gfp.h>     // Get Free Page flags
#include <linux/highmem.h> // For mapping memory

static void *shared_memory_buffer; // Pointer to our 4KB page
```
### 2. Allocate Memory in `__init`, Free in __exit
We are not using `static char message[256]` anymore. We need a real page.
```C
// Inside bridge_init:
// GFP_KERNEL: Allocate normal kernel ram. get_zeroed_page clears it for us.
shared_memory_buffer = (void *)get_zeroed_page(GFP_KERNEL);
if (!shared_memory_buffer) {
    // Handle error...
}
```
Also free in `__exit`:
```C
free_page((unsigned long)shared_memory_buffer);
```
### 3. `mmap` Operation
This connects the user's "Virtual Memory" to our "Physical Memory".
```C
static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long pfn;

    // Convert out kernel address to a "Page Frame Number" (Physical Address ID)
    pfn = virt_to_phys(shared_memory_buffer) >> PAGE_SHIFT;

    // Remap the user's requested memory range (vma) to our physical page
    if (remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end - vma->vm_start, vma->vm_page_prot))
    {
        return -EAGAIN;
    }

    return 0;
}
```

### 4. Update `fops`
```C
static struct file_operations fops = {
    // ... open, release ...
    .mmap = dev_mmap, // Add this line!
};
```

### 5. Add Tester
We cannot test `mmap` with `cat` or `echo`. We need a C program (User Space).

Add this **User Space Tester (`test_mmap.c`)**:
```C
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>    // Open
#include <unistd.h>   // Close
#include <sys/mman.h> // mmap
#include <string.h>
#include <errno.h>

#define DEVICE "/dev/my_bridge"
#define PAGE_SIZE 4096

int main()
{
    int fd;
    char *shared_memory;

    // 1. Open the device
    fd = open(DEVICE, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device (Did you chmod 666?)");
        return -1;
    }

    // 2. The Magic: Ask kernel to map the driver's memory into OUR memory space
    // Arguments: NULL (let OS choose address), Size, Protection (Read/Write), Flags (Shared), File Desc, Offset
    shared_memory = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shared_memory == MAP_FAILED)
    {
        perror("mmap failed");
        close(fd);
        return -1;
    }

    printf("--- Zero-Copy Test ---\n");
    printf("1. Reading initial state from Kernel memory: '%s'\n", shared_memory);

    // 3. Write directly to memory (No write() syscall overhead!)
    printf("2. Writing to memory via pointer...\n");
    sprintf(shared_memory, "Success! This data skipped the CPU copy instructions!");

    // 4. Read it back immediately
    printf("3. Reading back from Kernel memory: '%s'\n", shared_memory);

    // Cleanup
    munmap(shared_memory, PAGE_SIZE);
    close(fd);
    return 0;
}
```

### 6. Compile and Test
1. `make`
2. `sudo insmod my_bridge.ko`
3. Compile the tester: `gcc test_mmap.c -o test_mmap`
4. Grant Permission: `sudo chmod 666 /dev/my_bridge`
5. Run the vertification: `./test_mmap`
6. Expected:
    ```
    --- Zero-Copy Test ---
    1. Reading initial state from Kernel memory: ''
    2. Writing to memory via pointer...
    3. Reading back from Kernel memory: 'Success! This data skipped the CPU copy instructions!'
    ```
