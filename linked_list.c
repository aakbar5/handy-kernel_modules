// An example of linked_list

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/list.h>

#define COUNT           5
#define NAME_LENGTH     50

static int param_count = COUNT;

struct object {
    char name[NAME_LENGTH];
    int id;
    struct list_head list; /* Import kernel linked list */
};

// Create a list head
static LIST_HEAD(object_list);

// Util function to create an object
struct object* create_node(int number) {

    struct object *node = kmalloc(sizeof(struct object), GFP_KERNEL);
    if (!node) {
        pr_err("Fail to allocate memory\n");
        return NULL;
    }

    // Fill fields of the structure
    sprintf(node->name, "obj=%d", number);
    node->id = number;
    INIT_LIST_HEAD(&node->list);

    return node;
}

//
// Module entry point
//
static int __init ll_init(void) {
    pr_info("ll: init\n");

    pr_info("Add nodes to the list...\n");
    {
        int idx;
        struct object *obj;
        for (idx = 0; idx < COUNT; ++idx) {
            obj = create_node(idx);
            if (obj) {

                // /* Add new node to the start of the list */
                // list_add(&obj->list, &object_list);

                /* Add new node to the end of the list */
                list_add_tail(&obj->list, &object_list);
            }
        }
    }

    // Test whether list is empty or not
    if (list_empty(&object_list) == 1) {
        pr_info("List is empty\n");
    }
    else {
        pr_info("List is not empty\n");
    }

    pr_info("Traverse the list...\n");
    {
        int idx = 1;
        struct list_head *list_head;
        struct object *entry;

        list_for_each(list_head, &object_list) {
            entry = list_entry(list_head, struct object, list);
            pr_info(" #%d: %s\n", idx, entry->name);
            idx++;
        }
    }

    pr_info("Delete all nodes from the list...\n");
    {
        struct list_head *list_head;
        struct list_head *list_tmp;
        struct object *entry;

        // NOTE: As we traversing list after deleting node
        //       so we have to use list_for_each_safe instead of
        //       list_for_each because list_for_each_safe saves
        //       next & previous pointers before remove an element
        //       from the list.
        list_for_each_safe(list_head, list_tmp, &object_list) {
            entry = list_entry(list_head, struct object, list);
            list_del(&entry->list);
            kfree(entry);
        }
    }

    // Test whether list is empty or not
    if (list_empty(&object_list) == 1) {
        pr_info("List is empty\n");
    }
    else {
        pr_info("List is not empty\n");
    }

    pr_info("Traverse the list...\n");
    {
        int idx = 1;
        struct list_head *list_head;
        struct object *entry;

        list_for_each(list_head, &object_list) {
            entry = list_entry(list_head, struct object, list);
            pr_info(" #%d: %s\n", idx, entry->name);
            idx++;
        }
    }

    return 0;
}

//
// Module exit point
//
static void __exit ll_exit(void) {
    pr_info("ll: exit\n");
}

//
// Setup module entry and exit points
//
module_init(ll_init);
module_exit(ll_exit);

//
// Setup module parameters
//
module_param(param_count, int, S_IRUGO);
MODULE_PARM_DESC(param_count, "Number of nodes in a list -- (Read only)");

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("ll: linked list example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
