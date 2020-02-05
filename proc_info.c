// Show list of processes

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/rwlock.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>

struct proinfo {
    int id;
    char name[16];
	char state[20];
	char real_start_time[50]; /* Boot based time in nsecs: */
    struct list_head list;
};

// Create a list head
static LIST_HEAD(proinfo_list);

// Util function to create an object
struct proinfo* create_node(void) {
    struct proinfo *node = kmalloc(sizeof(struct proinfo), GFP_KERNEL);
    if (!node) {
        pr_err("Fail to allocate memory\n");
        return NULL;
    }

    INIT_LIST_HEAD(&node->list);
    return node;
}

// Util function to translate nseconds into wall clock
static void translate_time(char *string, u64 time_in_ns) {
	int secs = time_in_ns / 1000000000;
	sprintf(string, "%02d:%02d:%02d", secs / 3600, (secs / 60) % 60, secs % 60);
}

// Util function to translate process state
static void translate_state(char *string, long state) {
	/* -1 unrunnable, 0 runnable, >0 stopped: */
	if (state < 0)
		strcpy(string, "unrunnable");
	else if (state == TASK_RUNNING)
		strcpy(string, "running");
	else
	{
		strcpy(string, "Stopped[");
		if (state & TASK_INTERRUPTIBLE)
			strcat(string, "In");
		if (state & TASK_UNINTERRUPTIBLE)
			strcat(string, "Un");
		if (state & EXIT_DEAD)
			strcat(string, "Ed");
		if (state & EXIT_ZOMBIE)
			strcat(string, "Zo");
		if (state & TASK_PARKED)
			strcat(string, "Pa");
		if (state & TASK_DEAD)
			strcat(string, "De");
		if (state & TASK_WAKEKILL)
			strcat(string, "Wk");
		if (state & TASK_WAKING)
			strcat(string, "Wa");
		if (state & TASK_NOLOAD)
			strcat(string, "No");
		if (state & TASK_NEW)
			strcat(string, "Ne");
		strcat(string, "]");
	}
}

// Go through each process to gather process info
void parse(void) {
	struct task_struct *pro;
	struct proinfo *node;

	for_each_process(pro) {
        if (!pro)
			continue;

		node = create_node();
		if (!node)
			continue;

		node->id = pro->pid;
		translate_time(node->real_start_time, pro->real_start_time);
		translate_state(node->state, pro->state);
		strcpy(node->name, pro->comm);

		list_add_tail(&node->list, &proinfo_list);
	}
}

// Print info assoicated to each process
static void show(void) {
	struct list_head *list_head;
	struct proinfo *entry;

	pr_info("%4s %20s %20s %10s\n", "PID", "NAME", "STATE", "TIME");
	list_for_each(list_head, &proinfo_list) {
		entry = list_entry(list_head, struct proinfo, list);
		pr_info("%4d %20s %20s %10s\n", entry->id, entry->name, entry->state, entry->real_start_time);

		if (entry->id == current->pid) {
			int idx = 1;
        	struct vm_area_struct *vma;
			struct mm_struct *mm = current->mm;
			pr_info("        Code Section: 0x%016lx -- 0x%016lx\n", mm->start_code, mm->end_code);
			pr_info("        Data Section: 0x%016lx -- 0x%016lx\n", mm->start_data, mm->end_data);
			pr_info("        Stack:        0x%016lx\n", mm->start_stack);
			pr_info("        Task size:    0x%lx\n", mm->task_size);
			pr_info("        VMA Count:    %d\n", mm->map_count);
			for (vma = mm->mmap; vma; vma = vma->vm_next, ++idx) {
                printk("        VMA # %2d: 0x%016lx -- 0x%016lx\n", idx, vma->vm_start, vma->vm_end);
			}
		}
	}
}

//
// Module entry point
//
static int __init proinfo_init(void) {
    pr_info("proinfo_module: init\n");

	parse();
	show();

    return 0;
}

//
// Module exit point
//
static void __exit proinfo_exit(void) {
	struct list_head *list_head;
	struct list_head *list_tmp;
	struct proinfo *entry;

    pr_info("proinfo_module: exit\n");

	list_for_each_safe(list_head, list_tmp, &proinfo_list) {
		entry = list_entry(list_head, struct proinfo, list);
		list_del(&entry->list);
		kfree(entry);
	}
}

//
// Setup module entry and exit points
//
module_init(proinfo_init);
module_exit(proinfo_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("proinfo: show info related to processes");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("aakbar5 <16612387+aakbar5@users.noreply.github.com>");
