// An example of wait_queue

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

// Sleep time (mseconds)
static const unsigned long sleep_time = 1000 * 10;

// Structure having module related data
struct lockmod_info {
    int condition; /* Condition */
    struct work_struct work; /* Worker */
    struct wait_queue_head work_queue; /* Work queue */

} *g_lockmod_info;

// Callback to perform work
static void wq_work_handler(struct work_struct *work) {
    struct lockmod_info *info = container_of(work, struct lockmod_info, work);

    pr_info("wq_work_handler -- START\n");

    pr_info("wq_work_handler -- Doing working\n");
    msleep(sleep_time);

    pr_info("wq_work_handler -- Work is done\n");
    info->condition = 1;

    pr_info("wq_work_handler -- END\n");
    wake_up_interruptible_all(&info->work_queue);
}

//
// Module entry point
//
static int __init wq_init(void) {
    pr_info("wq: init\n");

    g_lockmod_info = kmalloc(sizeof(struct lockmod_info), GFP_KERNEL);
    if (!g_lockmod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    g_lockmod_info->condition = 0;
    init_waitqueue_head(&g_lockmod_info->work_queue);

    // Initialize work
    pr_info("Create work to be done...\n");
    {
        INIT_WORK(&g_lockmod_info->work, wq_work_handler);
    }

    // Schedule work
    pr_info("Create work to be done...\n");
    {
        schedule_work(&g_lockmod_info->work);
    }

    pr_info("Going to suspend until is not done...\n");
    {
        wait_event_interruptible(g_lockmod_info->work_queue, g_lockmod_info->condition != 0);
    }

    pr_info("Wow! work is complete...\n");
    pr_info("wq: Bye!\n");

    return 0;
}

//
// Module exit point
//
static void __exit wq_exit(void) {
    pr_info("wq: exit\n");

    if (!g_lockmod_info)
        return;

    kfree(g_lockmod_info);
    g_lockmod_info = NULL;
}

//
// Setup module entry and exit points
//
module_init(wq_init);
module_exit(wq_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("wq: wait_queue example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
