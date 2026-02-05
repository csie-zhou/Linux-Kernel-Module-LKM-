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