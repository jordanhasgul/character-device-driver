# Character Device Driver
A basic concurrent Linux loadable kernel module (LKM) character device driver.

## Description
A device driver is a kernel component that allows user space programs to interact with hardware devices via an associated interface referred to as a device file. In particular, a character device driver is a device driver that performs I/O operations in a byte stream i.e. read and write operations are performed sequentially, byte by byte.

This particular implementation of a character device driver provides a simple way of message passing and works as follows:

- Upon creation of the device driver, an empty list of messages is created.
- Writing to the device stores the message in kernel space and adds it to the list of messages. A message is only added to the message list if the following constraints are satisfied:
  - The size of the message to be added is less than 6KB (6 * 1024 bytes). 
  - The size of all messages, including the message to be added, does not surpass 4MB (4 * 1024 * 1024 bytes).
- Reading from the device removes and returns a single message from the message list, provided that the message list is not empty. Messages are read from/written to the message list in a LIFO (last in, first out) manner, thus, the message list essentially acts as a stack.
- Removing the device deallocates and removes the list of messages.

## Usage

**Prerequisite: To ensure that the kernel module can be built and correctly inserted into the kernel, root access to your system must be aquired. This is achieved by entering `sudo su` into the command line, then entering the correct password.**

To build the kernel module, open the command line and navigate to the project folder. Then, enter the `make` command. Once successfully built, we can then insert the kernel module by entering the following command into the command line:
```
sudo insmod charDeviceDriver.ko
```
Once inserted, enter `dmesg -w &` to view, both, the commands used to interact with the kernel module and any logs produced by the device driver. Then, to remove the kernel module, enter the following command into the command line:
```
sudo rmmod charDeviceDriver.ko
```
