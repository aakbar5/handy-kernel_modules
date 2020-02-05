#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>

static int param_integer = 500;
static int param_array[3] = { 500, 501, 502 };
static char *param_string = "helloworld";

static void show_settings(void) {
    pr_info("Helloworld_parameters:\n");
    pr_info(" - param_integer # %d\n", param_integer);
    pr_info(" - param_array # [%4d][%4d][%4d]\n", param_array[0], param_array[1], param_array[2]);
    pr_info(" - param_string # %s\n", param_string);
}

//
// Module entry point
//
static int __init helloworld_init(void) {
    pr_info("Helloworld_module: init\n");
    show_settings();
    return 0;
}

//
// Module exit point
//
static void __exit helloworld_exit(void) {
    pr_info("Helloworld_module: exit\n");
    show_settings();
}

//
// Setup module entry and exit points
//
module_init(helloworld_init);
module_exit(helloworld_exit);

module_param(param_integer, int, S_IRUGO);
module_param_array(param_array, int, NULL, S_IWUSR | S_IRUSR);
module_param(param_string, charp, S_IRUGO);

MODULE_PARM_DESC(param_integer, "Integer variable -- (Read only)");
MODULE_PARM_DESC(param_array, "Integer array[3] -- (Read/Write only)");
MODULE_PARM_DESC(param_string, "String -- (Read only)");

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Helloworld linux module with parameters");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
