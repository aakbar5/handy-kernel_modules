// An example of container_of

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/slab.h>

#define COUNT           5
#define NAME_LENGTH     50

static int param_count = COUNT;
static int param_idx = 1;

struct object {
    char name[NAME_LENGTH];
    int id;
};

struct group {
    struct object *obj1;
    struct object *obj2;
    int id;
};

static struct group array_of_groups[COUNT];

//
// Handy function to show group
//
static void show_group(const char* msg, struct group *grp) {
    if (grp) {
        pr_info("Group @ %s\n", msg);
        pr_info(" - Id # %d\n", grp->id);
        pr_info(" - Obj1 # %2d -- %s\n", grp->obj1->id, grp->obj1->name);
        pr_info(" - Obj2 # %2d -- %s\n", grp->obj2->id, grp->obj2->name);
    }
}

//
// Module entry point
//
static int __init co_init(void) {

    struct object *obj1, *obj2;
    int i;

    pr_info("co: init\n");

    for (i = 0; i < param_count; ++i) {

        // Allocate object # 1
        obj1 = kmalloc(sizeof(struct object), GFP_KERNEL);
        if (!obj1) {
            pr_err("Fail to allocate memory\n");
            return -ENOMEM;
        }

        sprintf(obj1->name, "obj1=%d", i);
        obj1->id = i + 20;

        // Allocate object # 2
        obj2 = kmalloc(sizeof(struct object), GFP_KERNEL);
        if (!obj2) {
            pr_err("Fail to allocate memory\n");
            return -ENOMEM;
        }

        sprintf(obj2->name, "obj2=%d", i);
        obj2->id = i + 10;

        // Save our objects
        array_of_groups[i].obj1 = obj1;
        array_of_groups[i].obj2 = obj2;
        array_of_groups[i].id = obj1->id + obj2->id;
    }

    if (param_idx >=0 && param_idx < param_count) {
        {
            struct group *grp = &array_of_groups[param_idx];
            show_group("index based", grp);
        }
        {
            int *ptr_id = &array_of_groups[param_idx].id;
            struct group *grp = container_of(ptr_id, struct group, id);
            show_group("container_of @ id", grp);
        }
        {
            struct object **p = &array_of_groups[param_idx].obj1;
            struct group *grp = container_of(p, struct group, obj1);
            show_group("container_of @ obj1", grp);
        }
    }

    return 0;
}

//
// Module exit point
//
static void __exit co_exit(void) {
    int i;

    pr_info("co: exit\n");
    for (i = 0; i < param_count; ++i) {
        kfree(array_of_groups[i].obj1);
        kfree(array_of_groups[i].obj2);
    }
}

//
// Setup module entry and exit points
//
module_init(co_init);
module_exit(co_exit);

//
// Setup module parameters
//
module_param(param_count, int, S_IRUGO);
MODULE_PARM_DESC(param_count, "Number of groups to be created -- (Read only)");

module_param(param_idx, int, S_IWUSR | S_IRUSR);
MODULE_PARM_DESC(param_idx, "Index of the group to be shown -- (Read/write only)");

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("co: container_of example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
