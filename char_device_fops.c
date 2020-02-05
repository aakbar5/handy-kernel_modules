// Create a character device with file ops

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

const size_t data_limit = 1024 * 2;
struct cddev_device {
    unsigned minor;
    unsigned count;
    char name[50];
    dev_t dev_num;
    struct cdev cdev;
    struct class *class;
    struct device *device;

    char *data;
} cddev_info = {
    .minor = 0,
    .count = 1,
    .name = "chdev"
};

static int cd_open(struct inode *inodep, struct file *filep) {
    struct cddev_device *dev = container_of(inodep->i_cdev, struct cddev_device, cdev);
    pr_info("cd: open\n");

    if (imajor(inodep) != MAJOR(dev->dev_num) || iminor(inodep) != MINOR(dev->dev_num)) {
        pr_err("Unable to find device\n");
        return -ENODEV;
    }

    if (dev->data == NULL) {
        dev->data = kzalloc(data_limit, GFP_KERNEL);
        if (IS_ERR(dev->data)) {
            pr_err("Unable to allocate memory for data\n");
            return PTR_ERR(dev->data);
        }
    }
    else {
        pr_info("  Memory is already allocated\n");
    }

    filep->private_data = dev;
    return 0;
}

static int cd_release(struct inode *inodep, struct file *filep) {
    struct cddev_device *dev = container_of(inodep->i_cdev, struct cddev_device, cdev);
    pr_info("cd: release\n");

    if (imajor(inodep) != MAJOR(dev->dev_num) || iminor(inodep) != MINOR(dev->dev_num)) {
        pr_err("Unable to find device\n");
        return -ENODEV;
    }

    pr_info("=== Data found from user ===\n");
    pr_info("%s", dev->data);
    pr_info("\n============================\n");

    return 0;
}

static int cd_flush(struct file *filep, fl_owner_t id) {
    // pr_info("cd: flush\n");
    (void)filep;
    (void)id;
    return 0;
}

static ssize_t cd_read(struct file *filep, char __user *buf, size_t count, loff_t *pos) {
    unsigned long ret;
    size_t ipos = *pos;

    struct cddev_device *dev = filep->private_data;
    if (IS_ERR(dev)) {
        pr_err("Unable to find private data\n");
        return PTR_ERR(dev);
    }

    pr_info("cd: read\n");

    if (ipos > data_limit)
        return -EINVAL;

    if (ipos + count > data_limit)
        count = data_limit - ipos;

    ret = copy_to_user(buf, dev->data + ipos, count);
    if (ret != 0) {
        pr_err("Unable to copy data to user (%ld -- %ld)\n", ret, count);
        return -EFAULT;
    }

    *pos += count;
    return count;
}

static ssize_t cd_write(struct file *filep, const char __user *buf, size_t count, loff_t *pos) {
    unsigned long ret;
    size_t ipos = *pos;

    struct cddev_device *dev = filep->private_data;
    if (IS_ERR(dev)) {
        pr_err("Unable to find private data\n");
        return PTR_ERR(dev);
    }

    pr_info("cd: write (%ld)\n", ipos);
    
    if (ipos > data_limit)
        return -EINVAL;

    if (ipos + count > data_limit)
        count = data_limit - ipos;

    ret = copy_from_user(dev->data + ipos, buf, count);
    if (ret != 0) {
        pr_err("Unable to copy data from user (%ld -- %ld)\n", ret, count);
        return -EFAULT;
    }

    *pos += count;
    return count;
}

//
// File operation implemented by the kernel module
//
static struct file_operations cddev_ops = {
    .owner = THIS_MODULE,
    .open = cd_open,
    .release = cd_release,
    .read = cd_read,
    .write = cd_write,
    .flush = cd_flush,
    .llseek = default_llseek /* Use default llseek implementation */
};

//
// Module entry point
//
static int __init cd_init(void) {
    int err = 0;
    int ret;

    pr_info("cd: init\n");

    /* Step # 1: Reverse device major and minor number */
    ret = alloc_chrdev_region(&cddev_info.dev_num, cddev_info.minor, cddev_info.count, cddev_info.name);
    if (ret < 0) {
        pr_err("Unable to allocate character device number\n");
        err = ret;
        goto err_err;
    }

    /* Step # 2: Create class for our device */
    cddev_info.class = class_create(THIS_MODULE, cddev_info.name);
    if (IS_ERR(cddev_info.class)) {
        pr_err("Unable to create class\n");
        err = PTR_ERR(cddev_info.class);
        goto err_chrdev;
    }

    /* Step # 3: Create cdevice */
    cdev_init(&cddev_info.cdev, &cddev_ops);

    /* Step # 4: Setup ownership of the device */
    cddev_info.cdev.owner = THIS_MODULE;
    cddev_info.cdev.ops = &cddev_ops;

    /* Step # 4: Add device to the system */
    ret = cdev_add(&cddev_info.cdev, cddev_info.dev_num, cddev_info.count);
    if (ret < 0) {
        pr_err("Unable to add cdev\n");
        err = ret;
        goto err_class;
    }

    /* Step # 5: Create a device and register it with sysfs */
    cddev_info.device = device_create(cddev_info.class, NULL, cddev_info.dev_num, NULL, cddev_info.name);
    if (IS_ERR(cddev_info.device)) {
        pr_err("Unable to create device\n");
        err = PTR_ERR(cddev_info.device);
        goto err_cdev;
    }

    pr_info("cd: device(/dev/%s) is create\n", cddev_info.name);
    pr_info(" - Major number # %d\n", MAJOR(cddev_info.dev_num));
    pr_info(" - Minor number # %d\n", MINOR(cddev_info.dev_num));

    return 0;

err_cdev:
    cdev_del(&cddev_info.cdev);

err_class:
    class_destroy(cddev_info.class);

err_chrdev:
    unregister_chrdev_region(cddev_info.dev_num, cddev_info.count);

err_err:
    return err;
}

//
// Module exit point
//
static void __exit cd_exit(void) {
    pr_info("cd: exit\n");

    device_destroy(cddev_info.class, cddev_info.dev_num);
    cdev_del(&cddev_info.cdev);
    class_destroy(cddev_info.class);
    unregister_chrdev_region(cddev_info.dev_num, cddev_info.count);
}

//
// Setup module entry and exit points
//
module_init(cd_init);
module_exit(cd_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("cd: A character device with file ops example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
