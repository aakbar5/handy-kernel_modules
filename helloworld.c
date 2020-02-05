#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

//
// Module entry point
//
static int __init helloworld_init(void) {
    pr_info("Helloworld_module: init\n");
    return 0;
}

//
// Module exit point
//
static void __exit helloworld_exit(void) {
    pr_info("Helloworld_module: exit\n");
}

//
// Setup module entry and exit points
//
module_init(helloworld_init);
module_exit(helloworld_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Helloworld linux module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
