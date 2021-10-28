// An example of percpu variables

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/cpu.h>

typedef unsigned int uint;

// Per-CPU variable (static)
static DEFINE_PER_CPU(uint, var_static) = 0;

// Per-CPU variable (allocated on the fly)
static uint __percpu *var_dynamic = 0;

//
// Module entry point
//
static int __init mod_init(void) {
    uint cpu;

    pr_info("mod: init\n");

    pr_info("smp_processor_id: %d\n", smp_processor_id()); // active cpu id
    pr_info("num_online_cpus: %d\n", num_online_cpus());

    // Allocate dynamic var
    var_dynamic = alloc_percpu(uint);

    pr_info("--- Variable (var_static) before modification\n");
    for_each_possible_cpu(cpu) {
        uint tmp = per_cpu(var_static, cpu);
		pr_info("  cpu # %d <> %d\n", cpu, tmp);
	}

    pr_info("--- Variable (var_dynamic) before modification\n");
    for_each_possible_cpu(cpu) {
        uint tmp = *per_cpu_ptr(var_dynamic, cpu);
		pr_info("  cpu # %d <> %d\n", cpu, tmp);
	}

    // Update variable for only current cpu
	get_cpu_var(var_static) = 786;
	put_cpu_var(var_static);

    this_cpu_inc(*var_dynamic);
    this_cpu_inc(*var_dynamic);
    this_cpu_inc(*var_dynamic);

    pr_info("\n============================================\n");
    pr_info("--- Variable (var_static) after modification\n");
    for_each_possible_cpu(cpu) {
        uint tmp = per_cpu(var_static, cpu);
		pr_info("  cpu # %d <> %d\n", cpu, tmp);
	}

    pr_info("--- Variable (var_dynamic) before modification\n");
    for_each_possible_cpu(cpu) {
        uint tmp = *per_cpu_ptr(var_dynamic, cpu);
		pr_info("  cpu # %d <> %d\n", cpu, tmp);
	}

    return 0;
}

//
// Module exit point
//
static void __exit mod_exit(void) {
    pr_info("mod: exit\n");
    free_percpu(var_dynamic);
}

//
// Setup module entry and exit points
//
module_init(mod_init);
module_exit(mod_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("mod: percpu example");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
