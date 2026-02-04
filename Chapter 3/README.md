# Chapter 3. Automation & Race Conditions
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
Modify the `__init` function to thos:
```C
```
