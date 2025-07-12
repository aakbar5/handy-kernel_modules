// repo: asadmalik5/handy-kernel_modules.git
// Power integration: A simple module to interact with power framework
// Ref: https://www.kernel.org/doc/html/v6.15/driver-api/pm/index.html

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm.h>

#define PI_DEVICE_NAME "pi_device"

//
// Device power management operations
//
static int pi_pm_prepare(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static void pi_pm_complete(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); }
static int pi_pm_suspend(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_resume(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_freeze(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_thaw(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_poweroff(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_restore(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_suspend_late(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_resume_early(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_freeze_late(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_thaw_early(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_poweroff_late(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_restore_early(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_suspend_noirq(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_resume_noirq(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_freeze_noirq(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_thaw_noirq(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_poweroff_noirq(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_restore_noirq(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_runtime_suspend(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_runtime_resume(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_pm_runtime_idle(struct device *dev) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }

static const struct dev_pm_ops pm_ops = {
	.prepare = pi_pm_prepare,
	.complete = pi_pm_complete,
	.suspend = pi_pm_suspend,
	.resume = pi_pm_resume,
	.freeze = pi_pm_freeze,
	.thaw = pi_pm_thaw,
	.poweroff = pi_pm_poweroff,
	.restore = pi_pm_restore,
	.suspend_late = pi_pm_suspend_late,
	.resume_early = pi_pm_resume_early,
	.freeze_late = pi_pm_freeze_late,
	.thaw_early = pi_pm_thaw_early,
	.poweroff_late = pi_pm_poweroff_late,
	.restore_early = pi_pm_restore_early,
	.suspend_noirq = pi_pm_suspend_noirq,
	.resume_noirq = pi_pm_resume_noirq,
	.freeze_noirq = pi_pm_freeze_noirq,
	.thaw_noirq = pi_pm_thaw_noirq,
	.poweroff_noirq = pi_pm_poweroff_noirq,
	.restore_noirq = pi_pm_restore_noirq,
	.runtime_suspend = pi_pm_runtime_suspend,
	.runtime_resume = pi_pm_runtime_resume,
	.runtime_idle = pi_pm_runtime_idle,
};

//
// Functions to used by platform_driver
//
static int pi_probe(struct platform_device *) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static void pi_remove(struct platform_device *) { pr_info("pi_module @ %s\n", __FUNCTION__); }
static void pi_shutdown(struct platform_device *) { pr_info("pi_module @ %s\n", __FUNCTION__); }
static int pi_suspend(struct platform_device *, pm_message_t state) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }
static int pi_resume(struct platform_device *) { pr_info("pi_module @ %s\n", __FUNCTION__); return 0; }

static struct platform_driver pm_driver = {
    .probe = pi_probe,
    .remove = pi_remove,
	.shutdown = pi_shutdown,
    .suspend = pi_suspend,
    .resume = pi_resume,
	.driver = {
        .name = PI_DEVICE_NAME,
        .pm = &pm_ops,
    },
};

// Platform device
static struct platform_device *pi_device;

//
// Module entry point
//
static int __init pi_init(void) {
    pr_info("pi_module: init\n");

	// Register driver
	int ret = platform_driver_register(&pm_driver);
	if (ret != 0) {
		pr_err("Failed to register driver\n");
		return ret;
	}

	// Register a minimal device without any resource of platform data
	pi_device = platform_device_register_simple(
        PI_DEVICE_NAME,      
        -1,                  
        NULL,                
        0                    
    );

    if (IS_ERR(pi_device)) {
        pr_err("Failed to register platform device\n");
        return PTR_ERR(pi_device);
    }

    pr_info("Platform device '%s' registered\n", PI_DEVICE_NAME);
	return ret;
}

//
// Module exit point
//
static void __exit pi_exit(void) {
    pr_info("pi_module: exit\n");
	platform_device_unregister(pi_device);
	return platform_driver_unregister(&pm_driver);
}

//
// Setup module entry and exit points
//
module_init(pi_init);
module_exit(pi_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Power intergration module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("asadmalik5 <16612387+asadmalik5@users.noreply.github.com>");
