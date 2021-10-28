KERNELSRC ?= /ws/linux-5.10.72
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-none-linux-gnu-
PWD ?= $(shell pwd)

# obj-m := helloworld.o
# obj-m := helloworld_probe.o
# obj-m := helloworld_param.o
# obj-m := proc_info.o

# obj-m := container_of.o
# obj-m := linked_list.o

# obj-m := kthread_simple.o
# obj-m := kthread_advanced.o

# obj-m := lock_mutex.o
# obj-m := lock_spin_lock.o
# obj-m := lock_rw_spin_lock.o
# obj-m := lock_seqlock.o
# obj-m := lock_semaphore.o
# obj-m := lock_rw_semaphore.o

# obj-m := char_device.o
# obj-m := char_device_fops.o

# obj-m := sysfs_simple.o
# obj-m := sysfs_dynamic.o

# obj-m := timer.o
# obj-m := timer_repetitive.o
# obj-m := timer_hr.o
# obj-m := timer_hr_repetitive.o

# obj-m := tasklet.o
# obj-m := wait_queue.o
# obj-m := work_queue_simple.o
# obj-m := work_queue_delayed.o

# obj-m := per_cpu_vars.o

obj-m := notify_chain_publisher.o notify_chain_consumer.o

.PHONY: all default
all default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELSRC) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELSRC) M=$(PWD) clean

.PHONY: install module_install
install module_install:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELSRC) M=$(PWD) module_install
