// An example of work queue

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

// Sleep time (mseconds)
static const unsigned long sleep_time = 1000 * 10;

// Structure having module related data
struct mod_info {
    struct work_struct work; /* Worker */
    struct workqueue_struct *work_queue; /* Work queue */

} *g_mod_info;

// Callback to perform work
static void wq_work_handler(struct work_struct *work) {
    struct mod_info *info = container_of(work, struct mod_info, work);

    pr_info("wq_work_handler -- START\n");

    pr_info("wq_work_handler -- Doing working\n");
    msleep(sleep_time);

    pr_info("wq_work_handler -- END\n");
    (void)info;
}

//
// Module entry point
//
static int __init wq_init(void) {
    int ret;

    pr_info("wq: init\n");

    g_mod_info = kmalloc(sizeof(struct mod_info), GFP_KERNEL);
    if (!g_mod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    // Create work queue
    pr_info("Create work queue...\n");
    g_mod_info->work_queue = create_workqueue("wq_test");
    if (IS_ERR(g_mod_info->work_queue)) {
        pr_err("Fail to create work queue\n");
        ret = PTR_ERR(g_mod_info->work_queue);
        goto err_free_mem;
    }

    //
    // create_workqueue macro is asking to create 1 workqueue instead of per cpu moreover
    // work queue is having sysfs (cat /sys/bus/workqueue/devices/wq_test/max_active) interface
    // which won't be visible by default. To enable sysfs interface you need to use
    // alloc_workqueue("wq_test",__WQ_LEGACY | WQ_MEM_RECLAIM | WQ_SYSFS, 1)
    //
    // create_singlethread_workqueue will create an ordere workqueue which can
    // work for only one work item at any given time.

    // Initialize work
    pr_info("Create work to be done...\n");
    {
        INIT_WORK(&g_mod_info->work, wq_work_handler);
    }

    // Queue up a work
    pr_info("Queue a work...\n");
    {
        queue_work(g_mod_info->work_queue, &g_mod_info->work);
    }

    pr_info("wq: init -- end\n");

    return 0;

err_free_mem:
    kfree(g_mod_info);
    g_mod_info = NULL;
    return ret;
}

//
// Module exit point
//
static void __exit wq_exit(void) {
    pr_info("wq: exit\n");

    if (!g_mod_info)
        return;

    // Cancel the work.
    // NOTE: flush_work can also be used here to force the execution of work.
    cancel_work_sync(&g_mod_info->work);
    destroy_workqueue(g_mod_info->work_queue);

    kfree(g_mod_info);
    g_mod_info = NULL;
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
MODULE_DESCRIPTION("wq: workqueue example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
