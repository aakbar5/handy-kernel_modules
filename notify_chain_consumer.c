#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>

// Link functions to register/unregister callback for notifier chain
extern int register_sample_notifier(struct notifier_block *nb);
extern int unregister_sample_notifier(struct notifier_block *nb);

// Callback to be called for notification
static int notification_callback(struct notifier_block *nb, unsigned long action, void *data)
{
    pr_info("Notify chain consumer: notification # 0x%lx\n", action);
    return NOTIFY_OK;
}
static struct notifier_block notifier = 
{
    .notifier_call = notification_callback,
    .priority = 0,
};

//
// Module entry point
//
static int __init nc_consumer_init(void) {
    int err;

    pr_info("Notify chain consumer: init\n");

    // Register for notification
	err = register_sample_notifier(&notifier);
	if (err)
	{
		pr_err("Notify chain consumer: failed to register notify chain\n");
		return -1;
	}

    return 0;
}

//
// Module exit point
//
static void __exit nc_consumer_exit(void) {
    pr_info("Notify chain consumer: exit\n");
    unregister_sample_notifier(&notifier);
}

//
// Setup module entry and exit points
//
module_init(nc_consumer_init);
module_exit(nc_consumer_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Notify chain: consumer module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
