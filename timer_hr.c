// An example of higher resolution timer (single shot)

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>

// Structure having module related data
struct trmod_info {
    struct hrtimer hr_timer;

} *g_trmod_info;

// Callback to perform work
static enum hrtimer_restart hr_timer_callback(struct hrtimer *timer) {
    struct trmod_info *info = container_of(timer, struct trmod_info, hr_timer);

    pr_info("hr_timer_callback -- START\n");

    pr_info("hr_timer_callback -- Doing working\n");

    pr_info("hr_timer_callback -- END\n");
    (void)info;

    return HRTIMER_NORESTART;

    // Use hrtimer_forward API with HRTIMER_RESTART to set next expiration each time
}

//
// Module entry point
//
static int __init hr_init(void) {
    pr_info("hr: init\n");

    g_trmod_info = kmalloc(sizeof(struct trmod_info), GFP_KERNEL);
    if (!g_trmod_info) {
        pr_err("Fail to allocate memory\n");
        return -ENOMEM;
    }

    pr_info("Setup timer...\n");
    hrtimer_init(&g_trmod_info->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    g_trmod_info->hr_timer.function = hr_timer_callback;

    pr_info("Start timer...\n");
    hrtimer_start(&g_trmod_info->hr_timer, ms_to_ktime(5), HRTIMER_MODE_REL);

    return 0;
}

//
// Module exit point
//
static void __exit hr_exit(void) {
    pr_info("hr: exit\n");

    if (!g_trmod_info)
        return;

    pr_info("Delete timer...\n");
    hrtimer_cancel(&g_trmod_info->hr_timer);
    kfree(g_trmod_info);
}

//
// Setup module entry and exit points
//
module_init(hr_init);
module_exit(hr_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("hr: timer example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
