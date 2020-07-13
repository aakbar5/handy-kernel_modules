// Kernel module to create sysfs for static data

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>

const static char* module_name = "test";

struct ms_module {
    struct device *device;
    struct kobject *kobj;
} g_module;

// Variables expose via sysfs
static int readonly = 100;
static int readwrite = 0;

// Function to handle show/store via sysfs
static ssize_t readonly_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    return sprintf(buf, "%u\n", readonly);
}

static ssize_t readwrite_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", readwrite);
}

static ssize_t readwrite_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%du", &readwrite);
    pr_info("readwrite = %d\n", readwrite);
    return count;
}

// Setup attributes of the interface expose via sysfs
static struct kobj_attribute read_only_attr = __ATTR_RO(readonly);
static struct kobj_attribute readwrite_attr = __ATTR_RW(readwrite);
static struct attribute *k_attrs[] = {
    &read_only_attr.attr,
    &readwrite_attr.attr,
    NULL,
};

static struct attribute_group k_grp = {
    .name  = "root",
    .attrs = k_attrs,
};

//
// Module entry point
//
static int __init ms_init(void) {
    int ret;

    pr_info("cd: init\n");

    /* Create entry in sysfs (/sys/<module_name>) */
    g_module.kobj = kobject_create_and_add(module_name, kernel_kobj->parent);
    if (IS_ERR(g_module.kobj)) {
        pr_err("Unable to create kobject for sysfs\n");
        ret = -EIO;
        return ret;
    }

    /* Add attributes in sysfs */
    ret = sysfs_create_group(g_module.kobj, &k_grp);
    if (ret != 0) {
        pr_err("Unable to create sysfs group\n");
        goto err_kobject;
    }

    pr_info("Sysfs interface # /sys/%s/%s\n", module_name, k_grp.name);

    return 0;

err_kobject:
    kobject_put(g_module.kobj);
    return ret;
}

//
// Module exit point
//
static void __exit ms_exit(void) {
    pr_info("ms: exit\n");

    sysfs_remove_group(g_module.kobj, &k_grp);
    kobject_put(g_module.kobj);
}

//
// Setup module entry and exit points
//
module_init(ms_init);
module_exit(ms_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("ms: A module with sysfs");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
