// An example of seqlock

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/seqlock.h>

// Structure having module related data
struct mod_info {
    struct task_struct *read_task_1;
    struct task_struct *read_task_2;
    struct task_struct *write_task;
    unsigned int msecs_read;
    unsigned int msecs_write;

    u64 variable;       /* variable to be protected via lock */
    seqlock_t lock;

} *g_mod_info;

// Callback to perform work in the context of kthread
static int mod_read_callback(void *data) {
    struct mod_info *info = (struct mod_info *)data;

    /* kthread has associated struct task_struct and kernel's current is set to it */
    pr_info("mod_read_callback (%d -- %s) -- START\n", current->pid, current->comm);

    while (kthread_should_stop() == false) {

        unsigned int seq_no;
        do
        {
            seq_no = read_seqbegin(&info->lock);
            {
                pr_info("mod_read_callback (%d -- %s) -- work (%lld)\n", current->pid, current->comm, g_mod_info->variable);

                // NOTE: spin lock disable preemption so can't call sleep
                // msleep_interruptible(info->msecs_read);
            }
        } while (read_seqretry(&info->lock, seq_no));

        msleep_interruptible(info->msecs_read);
    }

    pr_info("mod_read_callback (%d -- %s) -- END\n", current->pid, current->comm);
    return 0;
}

// Callback to perform work in the context of kthread
static int mod_write_callback(void *data) {
    struct mod_info *info = (struct mod_info *)data;

    pr_info("mod_write_callback (%d -- %s) -- START\n", current->pid, current->comm);

    while (kthread_should_stop() == false) {
        write_seqlock(&info->lock);
        {
            pr_info("mod_write_callback (%d -- %s) -- work (%lld)\n", current->pid, current->comm, g_mod_info->variable);
            ++g_mod_info->variable;

            // NOTE: spin lock disable preemption so can't call sleep
            // msleep_interruptible(info->msecs_write);
        }
        write_sequnlock(&info->lock);
        msleep_interruptible(info->msecs_write);
    }

    pr_info("mod_write_callback (%d -- %s) -- END\n", current->pid, current->comm);
    return 0;
}

//
// Module entry point
//
static int __init mod_init(void) {
    int ret;

    pr_info("mod: init\n");

    g_mod_info = kmalloc(sizeof(struct mod_info), GFP_KERNEL);
    if (!g_mod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    g_mod_info->variable = 0;
    g_mod_info->msecs_read = 1000 * 2;
    g_mod_info->msecs_write = 1000 * 3;

    pr_info("Setup seqlock...\n");
    seqlock_init(&g_mod_info->lock);

    pr_info("Setup kthread (reader # 1)...\n");
    g_mod_info->read_task_1 = kthread_create(mod_read_callback, g_mod_info, "read_thread_1");
    if (IS_ERR(g_mod_info->read_task_1)) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_mod_info->read_task_1);
        goto err_free_mem;
    }

    pr_info("Setup kthread (reader # 2)...\n");
    g_mod_info->read_task_2 = kthread_create(mod_read_callback, g_mod_info, "read_thread_2");
    if (IS_ERR(g_mod_info->read_task_2)) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_mod_info->read_task_2);
        goto err_free_mem;
    }

    pr_info("Setup kthread (writer)...\n");
    g_mod_info->write_task = kthread_create(mod_write_callback, g_mod_info, "write_thread");
    if (IS_ERR(g_mod_info->write_task)) {
        pr_err("Fail to create kthread\n");
        ret = PTR_ERR(g_mod_info->write_task);
        goto err_free_mem;
    }

    // Above created kthread is not running yet
    pr_info("Start kthreads...\n");
    ret = wake_up_process(g_mod_info->read_task_1);
    pr_err(" - Read task # 1: # %d\n", ret); // 1 means kthread is started; 0 means already running

    ret = wake_up_process(g_mod_info->read_task_2);
    pr_err(" - Read task # 2: # %d\n", ret); // 1 means kthread is started; 0 means already running

    ret = wake_up_process(g_mod_info->write_task);
    pr_err(" - Write task: # %d\n", ret); // 1 means kthread is started; 0 means already running

    return 0;

err_free_mem:
    kfree(g_mod_info);
    g_mod_info = NULL;
    return ret;
}

//
// Module exit point
//
static void __exit mod_exit(void) {
    pr_info("mod: exit\n");

    if (!g_mod_info)
        return;

    kthread_stop(g_mod_info->write_task);
    kthread_stop(g_mod_info->read_task_1);
    kthread_stop(g_mod_info->read_task_2);
    kfree(g_mod_info);
    g_mod_info = NULL;
}

//
// Setup module entry and exit points
//
module_init(mod_init);
module_exit(mod_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("mod: seqlock example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
