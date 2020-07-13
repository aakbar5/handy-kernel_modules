#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>

//
// Module entry point
//
static int helloworld_probe(struct platform_device *pdev) {
    pr_info("Helloworld_module: probe\n");
    return 0;
}

//
// Module exit point
//
static int helloworld_remove(struct platform_device *pdev) {
    pr_info("Helloworld_module: remove\n");
    return 0;
}

// Need to add following in DTS file to confirm
// that probe has been invoked the linux kernel.
// helloworld {
//     compatible = "helloworld,helloworld-probe";
//     status = "ok";
// };
static const struct of_device_id hellworld_driver_match_table[] = {
    { .compatible = "helloworld,helloworld-probe" },
    {}
};
MODULE_DEVICE_TABLE(of, hellworld_driver_match_table);

static struct platform_driver hellworld_driver = {
    .probe  = helloworld_probe,
    .remove = helloworld_remove,
    .driver = {
        .name   = "helloworld-probe",
        .of_match_table = of_match_ptr(hellworld_driver_match_table),
    }
};
module_platform_driver(hellworld_driver);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("Helloworld linux module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
