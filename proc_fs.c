// repo: asadmalik5/handy-kernel_modules.git
// Kernel module to setup proc fs

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define pfs_dir_name "pfs"
#define pfs_file_name "node"

static struct proc_dir_entry *pfs_dir;
static struct proc_dir_entry *pfs_file;

static const char fixed_msg[] = "procfs is saying hello\n";
static const size_t fixed_msg_len = sizeof(fixed_msg);

#define USER_DATA_LEN   10
static char user_data[USER_DATA_LEN] = "";

static int pfs_open(struct inode *, struct file *) {
	pr_info("pfs: open\n");
	return 0;
}

static ssize_t pfs_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	pr_info("pfs: read\n");

	if (*ppos > 0) {
		return 0; // EOF
	}

	if (copy_to_user(buf, fixed_msg, fixed_msg_len)) {
		pr_err("Unable to copy data to user\n");
		return -EFAULT;
	}

	*ppos = fixed_msg_len;
	return fixed_msg_len;
}

static ssize_t pfs_write(struct file *, const char __user *from, size_t count, loff_t *ppos) {
	// pr_info("pfs: write\n");

	size_t len = count;

	if (len > USER_DATA_LEN) {
		len = USER_DATA_LEN - 1;
	}

	if (copy_from_user(user_data, from, len)) {
		pr_err("Unable to copy data from user\n");
		return 0;
	}

	pr_info("User passed data: {%s}(len=%ld)\n", user_data, len);
	return len;
}

//
// File operation implemented by the kernel module
//
static struct proc_ops pfs_ops = {
	.proc_open = pfs_open,
	.proc_read = pfs_read,
	.proc_write = pfs_write,
};

//
// Module entry point
//
static int __init pfs_init(void) {
	pr_info("pfs: init\n");

	/* Create a directory in /proc */
	pfs_dir = proc_mkdir(pfs_dir_name, NULL);
	if (pfs_dir == NULL) {
		pr_err("Error creating proc entry");
		return -ENOMEM;
	}

	/* Create a device node under above created directory */
	/* proc_create_data is another API which can be used if store and access is private */
	pfs_file = proc_create(pfs_file_name, 0644, pfs_dir, &pfs_ops);
	if (pfs_file == NULL) {
		pr_err("Error:Could not initialize /proc/%s\n", pfs_file_name);
		return -ENOMEM;
	}

	pr_info("/proc/%s/%s created\n", pfs_dir_name, pfs_file_name);
	return 0;
}

//
// Module exit point
//
static void __exit pfs_exit(void) {
	pr_info("pfs: exit\n");

	proc_remove(pfs_file);
	proc_remove(pfs_dir);
}

//
// Setup module entry and exit points
//
module_init(pfs_init);
module_exit(pfs_exit);

//
// Setup module info
//
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("pfs: A module with proc_fs");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("asadmalik5 <16612387+asadmalik5@users.noreply.github.com>");
