#ifndef CHAR_DEVICE_DRIVER_H
#define CHAR_DEVICE_DRIVER_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "chardev"

#define MAX_MESSAGE_SIZE 6 * 1024
#define TOTAL_MESSAGES_SIZE_LIMIT 4 * 1024 * 1024

static int majorNumber;

struct messageListNode
{
	int messageLength;
	char* message;
	struct list_head list;
};
static struct list_head messageList = LIST_HEAD_INIT(messageList);
static unsigned int messagesSize;

static int initCharDriver(void);
module_init(initCharDriver);

static void exitCharDevice(void);
module_exit(exitCharDevice);

static int charDeviceOpen(struct inode*, struct file*);
static int charDeviceRelease(struct inode*, struct file*);
static ssize_t charDeviceRead(struct file*, char*, size_t, loff_t*);
static ssize_t charDeviceWrite(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = { .open = charDeviceOpen, .release = charDeviceRelease,
				       .read = charDeviceRead, .write = charDeviceWrite };

#endif






