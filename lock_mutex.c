// An example of mutex

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/mutex.h>

// Structure having module related data
struct lockmod_info {
    struct task_struct *read_task;
    struct task_struct *write_task;
    unsigned int msecs_read;
    unsigned int msecs_write;

    u64 variable;       /* variable to be protected via lock */
    struct mutex lock;

} *g_lockmod_info;

// Callback to perform work in the context of kthread
static int lockmod_read_callback(void *data) {
    struct lockmod_info *info = (struct lockmod_info *)data;

    pr_info("lockmod_read_callback -- START\n");

    while (kthread_should_stop() == false) {
        mutex_lock(&info->lock);
        {
            pr_info("lockmod_read_callback  -- Doing working (%lld)\n", g_lockmod_info->variable);
            msleep_interruptible(info->msecs_read);
        }
        mutex_unlock(&info->lock);
    }

    pr_info("lockmod_read_callback -- END\n");
    return 0;
}

// Callback to perform work in the context of kthread
static int lockmod_write_callback(void *data) {
    struct lockmod_info *info = (struct lockmod_info *)data;

    pr_info("lockmod_write_callback -- START\n");

    while (kthread_should_stop() == false) {
        mutex_lock(&info->lock);
        {
            ++g_lockmod_info->variable;
            // pr_info("lockmod_write_callback -- Doing working (%lld)\n", g_lockmod_info->variable);
            msleep_interruptible(info->msecs_write);
        }
        mutex_unlock(&info->lock);
    }

    pr_info("lockmod_write_callback -- END\n");
    return 0;
}

//
// Module entry point
//
static int __init lockmod_init(void) {
    int ret;

    pr_info("lockmod: init\n");

    g_lockmod_info = kmalloc(sizeof(struct lockmod_info), GFP_KERNEL);
    if (!g_lockmod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    g_lockmod_info->variable = 0;
    g_lockmod_info->msecs_read = 1000 * 2;
    g_lockmod_info->msecs_write = 1000 * 1;

    pr_info("Setup mutex...\n");
    mutex_init(&g_lockmod_info->lock);

    pr_info("Setup kthread...\n");
    g_lockmod_info->read_task = kthread_create(lockmod_read_callback, g_lockmod_info, "read_thread");
    if (IS_ERR(g_lockmod_info->read_task)) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_lockmod_info->read_task);
        goto err_free_mem;
    }

    g_lockmod_info->write_task = kthread_create(lockmod_write_callback, g_lockmod_info, "write_thread");
    if (IS_ERR(g_lockmod_info->write_task)) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_lockmod_info->write_task);
        goto err_free_mem;
    }

    // Above created kthread is not running yet
    pr_info("Start kthreads...\n");
    ret = wake_up_process(g_lockmod_info->read_task);
    pr_err(" - Read task: # %d\n", ret); // 1 means kthread is started; 0 means already running

    ret = wake_up_process(g_lockmod_info->write_task);
    pr_err(" - Write task: # %d\n", ret); // 1 means kthread is started; 0 means already running

    return 0;

err_free_mem:
    kfree(g_lockmod_info);
    g_lockmod_info = NULL;
    return ret;
}

//
// Module exit point
//
static void __exit lockmod_exit(void) {
    pr_info("lockmod: exit\n");

    if (!g_lockmod_info)
        return;

    kthread_stop(g_lockmod_info->write_task);
    kthread_stop(g_lockmod_info->read_task);
    mutex_destroy(&g_lockmod_info->lock);
    kfree(g_lockmod_info);
    g_lockmod_info = NULL;
}

//
// Setup module entry and exit points
//
module_init(lockmod_init);
module_exit(lockmod_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("lockmod: mutex example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
