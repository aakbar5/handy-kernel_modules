// An example of timer
// A repetitive timer

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>

// Structure having module related data
struct trmod_info {
    struct timer_list trmod_timer;

} *g_trmod_info;

// Callback to perform work
static void timer_callback(struct timer_list *timer) {
    struct trmod_info *info = container_of(timer, struct trmod_info, trmod_timer);

    pr_info("trmod: timer_callback -- called (repetitive)\n");

    // Reenable timer
    mod_timer(&info->trmod_timer, jiffies + msecs_to_jiffies(250));
}

//
// Module entrmody point
//
static int __init trmod_init(void) {
    pr_info("trmod: init\n");

    g_trmod_info = kmalloc(sizeof(struct trmod_info), GFP_KERNEL);
    if (!g_trmod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    pr_info("trmod: Setup timer...\n");
    timer_setup(&g_trmod_info->trmod_timer, timer_callback, 0);

    pr_info("trmod: Setup timeout...\n");
    mod_timer(&g_trmod_info->trmod_timer, jiffies + msecs_to_jiffies(250));

    return 0;
}

//
// Module exit point
//
static void __exit trmod_exit(void) {
    pr_info("trmod: exit\n");

    if (!g_trmod_info)
        return;

    pr_info("trmod: Delete timer...\n");
    del_timer(&g_trmod_info->trmod_timer);
    kfree(g_trmod_info);
    g_trmod_info = NULL;
}

//
// Setup module entrmody and exit points
//
module_init(trmod_init);
module_exit(trmod_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("trmod: timer example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
