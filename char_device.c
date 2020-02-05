// Create a character device

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>

struct cddev_device {
    unsigned minor;
    unsigned count;
    char name[50];
    dev_t dev_num;
    struct class *class;
    struct cdev *cdev;
    struct device *device;
} cddev_info = {
    .minor = 0,
    .count = 1,
    .name = "chdev"
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
    cddev_info.cdev = cdev_alloc();
    if (IS_ERR(cddev_info.cdev)) {
        pr_err("Unable to allocate cdev\n");
        err = PTR_ERR(cddev_info.cdev);
        goto err_class;
    }

    /* Step # 4: Setup ownership of the device */
    cddev_info.cdev->owner = THIS_MODULE;
    cddev_info.cdev->ops = NULL;

    /* Step # 4: Add device to the system */
    ret = cdev_add(cddev_info.cdev, cddev_info.dev_num, cddev_info.count);
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
    cdev_del(cddev_info.cdev);

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
    cdev_del(cddev_info.cdev);
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
MODULE_DESCRIPTION("cd: A character device example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
