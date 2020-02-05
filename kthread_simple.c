// An example of kernel thread 

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>

// Structure having module related data
struct kt_info {
    struct task_struct *task;
    unsigned int msecs;

} *g_lockmod_info;

// Callback to perform work in the context of kthread
static int kt_callback(void *data) {
    struct kt_info *info = (struct kt_info *)data;

    pr_info("kt_callback -- START\n");

    while (kthread_should_stop() == false) {
        pr_info("kt_callback -- Doing working (%lld)\n", jiffies_64);
        msleep_interruptible(info->msecs);
    }
    
    pr_info("kt_callback -- END\n");
    return 0;
}

//
// Module entry point
//
static int __init kt_init(void) {
    int ret;
    
    pr_info("kt: init\n");

    g_lockmod_info = kmalloc(sizeof(struct kt_info), GFP_KERNEL);
    if (!g_lockmod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    g_lockmod_info->msecs = 1000 * 2;

    pr_info("Setup kthread...\n");
    g_lockmod_info->task = kthread_create(kt_callback, g_lockmod_info, "kt_thread");
    if (IS_ERR(g_lockmod_info->task)) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_lockmod_info->task);
        goto err_free_mem;
    }

    // Above created kthread is not running yet
    pr_info("Start kthread...\n");
    ret = wake_up_process(g_lockmod_info->task);
    pr_err(" - Task: # %d\n", ret); // 1 means kthread is started; 0 means already running

    return 0;

err_free_mem:
    kfree(g_lockmod_info);
    g_lockmod_info = NULL;
    return ret;    
}

//
// Module exit point
//
static void __exit kt_exit(void) {
    pr_info("kt: exit\n");

    if (!g_lockmod_info)
        return;

    kthread_stop(g_lockmod_info->task);
    kfree(g_lockmod_info);
    g_lockmod_info = NULL;
}

//
// Setup module entry and exit points
//
module_init(kt_init);
module_exit(kt_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("kt: kthread example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
