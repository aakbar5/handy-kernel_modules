// An example of Tasklet

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

// Data to tasklet
static char tl_data[] = "Tasklet data";

// Callback for tasklet
static void tl_tasklet_func(unsigned long data) {
    pr_info("tl_tasklet_func: %s\n", (char *)data);
}

// Tasklet
DECLARE_TASKLET(tl_tasklet, tl_tasklet_func, (unsigned long)tl_data);

//
// Module entry point
//
static int __init tl_init(void) {
    pr_info("tl: init\n");
    tasklet_schedule(&tl_tasklet);

    return 0;
}

//
// Module exit point
//
static void __exit tl_exit(void) {
    pr_info("tl: exit\n");
    tasklet_kill(&tl_tasklet);
}

//
// Setup module entry and exit points
//
module_init(tl_init);
module_exit(tl_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("tl: Tasklet example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
