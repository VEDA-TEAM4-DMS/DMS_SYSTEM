#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/string.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <linux/interrupt.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/input.h>

#include <linux/wait.h>
#include <linux/workqueue.h>

#include <linux/gpio.h>

#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/err.h>

/* GPIO PIN definition as RPi.HAT.A4 shield board */

//LED
#define PIN_LED1	(17)
