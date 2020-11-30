#include "charDeviceDriver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jordan Hasgul");
MODULE_DESCRIPTION("A basic concurrent Linux LKM character device driver");

DEFINE_MUTEX(devLock);

static int __init initCharDriver(void)
{
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	
	if (majorNumber < 0)
	{
		printk(KERN_ALERT "Error: unable to register device driver with major number %d.\n", majorNumber);
		return majorNumber;
	}

	printk(KERN_INFO "Log: device driver was installed successfully.\n");

	printk(KERN_INFO "Info: to create a device file, use 'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, majorNumber);
	printk(KERN_INFO "Info: to write to the device file, use 'echo <message> > /dev/%s'.\n", DEVICE_NAME);
	printk(KERN_INFO "Info: to read from the device file, use 'head -n 1 < /dev/%s'.\n", DEVICE_NAME);
	printk(KERN_INFO "Info: to delete the device file, use 'rm -rf %s'.\n", DEVICE_NAME);

	return 0;
}
static void __exit exitCharDevice(void)
{
	struct list_head* pos;
	struct list_head* temp;

	mutex_lock(&devLock);

	list_for_each_safe(pos, temp, &messageList)
	{
		struct messageListNode* mnode = list_entry(pos, struct messageListNode, list);
		list_del(pos);

		kfree(mnode->message);
		kfree(mnode);
	}

	mutex_unlock(&devLock);

	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "Log: device driver was removed successfully.\n");
}

static int charDeviceOpen(struct inode* inode, struct file* file)
{
	try_module_get(THIS_MODULE);
	printk(KERN_INFO "Log: device driver was opened by a process.\n");
	return 0;
}
static int charDeviceRelease(struct inode* inode, struct file* file)
{
	module_put(THIS_MODULE);
	printk(KERN_INFO "Log: device driver was closed by a process.\n");
	return 0;
}

static ssize_t charDeviceRead(struct file* file, char* buffer, size_t length, loff_t* offset)
{
	mutex_lock(&devLock);

	if (list_empty(&messageList))
	{
		printk(KERN_ALERT "Error: message list is empty.\n");
		mutex_unlock(&devLock);

		return -EAGAIN;
	}

	printk(KERN_INFO "Log: begun reading from the device driver.\n");

	struct messageListNode* mnode = list_first_entry(&messageList, struct messageListNode, list);
	
	copy_to_user(buffer, mnode->message, mnode->messageLength);
	length = mnode->messageLength;

	list_del(&mnode->list);
	messagesSize -= length;

	kfree(mnode->message);
	kfree(mnode);

	printk(KERN_INFO "Log: finished reading from the device driver.\n");
	
	mutex_unlock(&devLock);
	
	return length;
}
static ssize_t charDeviceWrite(struct file* file, const char* buffer, size_t length, loff_t* offset)
{
	if (length > MAX_MESSAGE_SIZE)
	{
		printk(KERN_ALERT "Error: message size exceeds the maximum message size of %d.\n", MAX_MESSAGE_SIZE);
		return -EINVAL;
	}

	mutex_lock(&devLock);

	if (messagesSize + length > TOTAL_MESSAGES_SIZE_LIMIT)
	{
		printk(KERN_ALERT "Error: message list would exceed %d bytes.\n", TOTAL_MESSAGES_SIZE_LIMIT);
		mutex_unlock(&devLock);	

		return -EAGAIN;
	}

	struct messageListNode* mnode = kmalloc(sizeof(struct messageListNode), GFP_KERNEL);
	if (mnode != NULL)
	{
		mnode->message = kmalloc(length * sizeof(char), GFP_KERNEL);
		if (mnode->message != NULL)
		{
			printk(KERN_INFO "Log: begun writing to the device driver.\n");

			mnode->messageLength = length;
			copy_from_user(mnode->message, buffer, mnode->messageLength);

			list_add(&mnode->list, &messageList);
			messagesSize += mnode->messageLength;

			printk(KERN_INFO "Log: finished writing to the device driver.\n");

			mutex_unlock(&devLock);

			return length;
		}
	}

	printk(KERN_ALERT "Error: memory allocation failed.\n");
	mutex_unlock(&devLock);

	return -ENOMEM;
}