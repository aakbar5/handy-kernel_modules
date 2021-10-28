#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/timer.h>

#define NOTIFY_CHAIN_INIT       0x80U
#define NOTIFY_CHAIN_EXIT       0x81U
#define NOTIFY_CHAIN_FLAG_1     0x82U
#define NOTIFY_CHAIN_FLAG_2     0x83U

// Create notifier head to manage list of registered consumers
// NOTE: Except SRCU Notifier, all type of notification types
// can be simply managed by using different type or APIs to be called.
// For example for blocking notifier use, BLOCKING_NOTIFIER_HEAD and blocking_notifier_chain_register function later.
// Incase of SRCU Notifier, you need kthread which can be used to call consumer callbacks.
static ATOMIC_NOTIFIER_HEAD(nc_notifier);

// Timer which will be used to send notification
static struct timer_list nc_timer;

// Expose APIs to register/unregister notify chain consumers
int register_sample_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&nc_notifier, nb);
}
EXPORT_SYMBOL_GPL(register_sample_notifier);

int unregister_sample_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&nc_notifier, nb);
}
EXPORT_SYMBOL_GPL(unregister_sample_notifier);

// Send notification for consumers
static int call_consumers(unsigned long val)
{
    return atomic_notifier_call_chain(&nc_notifier, val, NULL);
}

// Callback to perform work
static void timer_callback(struct timer_list *timer) {
    static unsigned long flag = 0;

    if (flag & 0x1)
        call_consumers(NOTIFY_CHAIN_FLAG_1);
    else
        call_consumers(NOTIFY_CHAIN_FLAG_2);

    ++flag;

    // Reenable timer
    mod_timer(&nc_timer, jiffies + msecs_to_jiffies(500));
}

//
// Module entry point
//
static int __init nc_publisher_init(void) {
    pr_info("Notify chain publisher: init\n");

    call_consumers(NOTIFY_CHAIN_INIT);

    timer_setup(&nc_timer, timer_callback, 0);
    mod_timer(&nc_timer, jiffies + msecs_to_jiffies(250));

    return 0;
}

//
// Module exit point
//
static void __exit nc_publisher_exit(void) {
    pr_info("Notify chain publisher: exit\n");

    call_consumers(NOTIFY_CHAIN_EXIT);
    del_timer_sync(&nc_timer);
}

//
// Setup module entry and exit points
//
module_init(nc_publisher_init);
module_exit(nc_publisher_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Notify chain: publisher module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
