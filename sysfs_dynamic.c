// Kernel module to create sysfs for dynamic data

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>

#define LENGTH 3

const static char* module_name = "test";

struct kobj_ext {
    struct kobj_attribute kattr;
    u32 payload;
};

struct ms_module {
    struct kobject *kobj;
    struct attribute_group agrp;
    struct kobj_ext *objs[LENGTH];
    struct attribute *kattrs[LENGTH + 1];
} g_module;

// Function to handle show/store via sysfs
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    struct kobj_ext *e_kobj = container_of(attr, struct kobj_ext, kattr);
    return sprintf(buf, "%d\n", e_kobj->payload);
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    struct kobj_ext *e_kobj = container_of(attr, struct kobj_ext, kattr);
    sscanf(buf, "%du", &e_kobj->payload);
    pr_info("store = %d\n", e_kobj->payload);
    return count;
}

//
// Module entry point
//
static int __init ms_init(void) {
    int ret;
    int idx;

    pr_info("ms: init\n");

    /* Setup logic */
    for (idx = 0; idx < LENGTH; ++idx) {
        g_module.objs[idx] = kzalloc(sizeof(struct kobj_ext), GFP_KERNEL);
        if (!g_module.objs[idx]) {
            pr_err("Unable to alloc memory\n");
            ret = -ENOMEM;
            goto err_objs;
        }

        g_module.objs[idx]->payload = idx;
        g_module.objs[idx]->kattr.attr.name = kasprintf(GFP_KERNEL, "idx%d", idx);
        g_module.objs[idx]->kattr.attr.mode = S_IWUSR | S_IRUGO;
        g_module.objs[idx]->kattr.show = sysfs_show;
        g_module.objs[idx]->kattr.store = sysfs_store;
        g_module.kattrs[idx] = &g_module.objs[idx]->kattr.attr;
    }

    g_module.kattrs[LENGTH] = NULL;

    /* Create entry in sysfs (/sys/<module_name>) */
    g_module.kobj = kobject_create_and_add(module_name, kernel_kobj->parent);
    if (IS_ERR(g_module.kobj)) {
        pr_err("Unable to create kobject for sysfs\n");
        ret = -EIO;
        goto err_objs;
    }

    /* Prepare sysfs group */
    g_module.agrp.name = kasprintf(GFP_KERNEL, "%s", "root");
    g_module.agrp.attrs = g_module.kattrs;

    /* Add attributes in sysfs */
    ret = sysfs_create_group(g_module.kobj, &g_module.agrp);
    if (ret != 0) {
        pr_err("Unable to create sysfs group\n");
        goto err_kobject;
    }

    pr_info("Sysfs interface # /sys/%s/%s\n", module_name, g_module.agrp.name);

    return 0;

err_objs:
    for (idx = 0; idx < LENGTH - 1; ++idx) {
        if (g_module.objs[idx]) {
            kfree(g_module.objs[idx]);
        }
    }

err_kobject:
    kobject_put(g_module.kobj);
    return ret;
}

//
// Module exit point
//
static void __exit ms_exit(void) {
    pr_info("ms: exit\n");

    for (idx = 0; idx < LENGTH - 1; ++idx) {
        if (g_module.objs[idx]) {
            kfree(g_module.objs[idx]);
        }
    }

    sysfs_remove_group(g_module.kobj, &g_module.agrp);
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
