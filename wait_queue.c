// An example of wait_queue
// A recurring timer is called which  sets
// the wait_queue condition after sometimes.
// Once wait_queue is signalled, delete the
// timer.

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>

// Sleep time (mseconds)
static const unsigned long sleep_time = 1000;

// Structure having module related data
struct wq_info {
    int timer_count;
    int condition;
    struct timer_list timer;
    struct wait_queue_head wait_queue;

} *g_wq_info;

// Callback to perform work
static void timer_callback(struct timer_list *timer) {
    struct wq_info *info = container_of(timer, struct wq_info, timer);

    pr_info("wq: timer_callback -- called (timer_count: %d)\n", info->timer_count);
    info->timer_count += 1;
    if (info->timer_count > 10) {
        info->condition += 1;
        wake_up_interruptible_all(&info->wait_queue);
    }

    /* Re-enable timer */
    mod_timer(&info->timer, jiffies + msecs_to_jiffies(sleep_time * 2));
}

//
// Module entry point
//
static int __init wq_init(void) {
    pr_info("wq: init\n");

    g_wq_info = kmalloc(sizeof(struct wq_info), GFP_KERNEL);
    if (!g_wq_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    g_wq_info->timer_count = 0;
    g_wq_info->condition = 0;

    pr_info("wq: Init wait_queue...\n");
    init_waitqueue_head(&g_wq_info->wait_queue);

    pr_info("wq: Setup timer...\n");
    timer_setup(&g_wq_info->timer, timer_callback, 0);

    pr_info("wq: Setup timeout...\n");
    mod_timer(&g_wq_info->timer, jiffies + msecs_to_jiffies(sleep_time));

    pr_info("wq: Going to suspend until is not done...\n");
    {
        wait_event_interruptible(g_wq_info->wait_queue, g_wq_info->condition != 0);

        pr_info("wq: wait_queue is signalled...\n");

        pr_info("wq: delete timer...\n");
        del_timer(&g_wq_info->timer);
    }

    pr_info("wq: init -- done\n");

    return 0;
}

//
// Module exit point
//
static void __exit wq_exit(void) {
    pr_info("wq: exit\n");

    if (!g_wq_info)
        return;

    kfree(g_wq_info);
    g_wq_info = NULL;
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
