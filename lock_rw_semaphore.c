// An example of rw semaphore

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/rwsem.h>

// Structure having module related data
struct lockmod_info {
    struct task_struct *read_tasks[2];
    struct task_struct *write_task;
    unsigned int msecs_read;
    unsigned int msecs_write;

    u64 variable;       /* variable to be protected via lock */
    struct rw_semaphore lock;

} *g_lockmod_info;

// Callback to perform work in the context of kthread
static int lockmod_read_callback0(void *data) {
    struct lockmod_info *info = (struct lockmod_info *)data;

    pr_info("lockmod_read_callback0 -- START\n");

    while (kthread_should_stop() == false) {
        down_read(&info->lock);
        {
            pr_info("lockmod_read_callback0  -- Doing working (%lld)\n", g_lockmod_info->variable);
            msleep_interruptible(info->msecs_read);
        }
        up_read(&info->lock);
    }

    pr_info("lockmod_read_callback0 -- END\n");
    return 0;
}

// Callback to perform work
static int lockmod_read_callback1(void *data) {
    struct lockmod_info *info = (struct lockmod_info *)data;

    pr_info("lockmod_read_callback1 -- START\n");

    while (kthread_should_stop() == false) {
        down_read(&info->lock);
        {
            pr_info("lockmod_read_callback1  -- Doing working (%lld)\n", g_lockmod_info->variable);
            msleep_interruptible(info->msecs_read);
        }
        up_read(&info->lock);
    }

    pr_info("lockmod_read_callback1 -- END\n");
    return 0;
}

// Callback to perform work in the context of kthread
static int lockmod_write_callback(void *data) {
    struct lockmod_info *info = (struct lockmod_info *)data;

    pr_info("lockmod_write_callback -- START\n");

    while (kthread_should_stop() == false) {
        down_write(&info->lock);
        {
            ++g_lockmod_info->variable;
            msleep_interruptible(info->msecs_write);
        }
        up_write(&info->lock);
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

    pr_info("Setup rw-semaphore...\n");
    init_rwsem(&g_lockmod_info->lock);

    pr_info("Setup kthread...\n");
    g_lockmod_info->read_tasks[0] = kthread_create(lockmod_read_callback0, g_lockmod_info, "read_thread0");
    if (IS_ERR(g_lockmod_info->read_tasks[0])) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_lockmod_info->read_tasks[0]);
        goto err_free_mem;
    }

    g_lockmod_info->read_tasks[1] = kthread_create(lockmod_read_callback1, g_lockmod_info, "read_thread1");
    if (IS_ERR(g_lockmod_info->read_tasks[1])) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_lockmod_info->read_tasks[1]);
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
    ret = wake_up_process(g_lockmod_info->read_tasks[0]);
    pr_err(" - Read task # 0: %d\n", ret); // 1 means kthread is started; 0 means already running

    ret = wake_up_process(g_lockmod_info->read_tasks[1]);
    pr_err(" - Read task # 1: %d\n", ret); // 1 means kthread is started; 0 means already running

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
    kthread_stop(g_lockmod_info->read_tasks[0]);
    kthread_stop(g_lockmod_info->read_tasks[1]);
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
MODULE_DESCRIPTION("lockmod: R/W Semaphore example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
