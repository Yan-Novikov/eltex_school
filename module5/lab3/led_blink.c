#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/tty.h>               /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>                /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>    /* For vc_cons */
#include <linux/vt_kern.h>

#define BLINK_DELAY   HZ/5
#define RESTORE_LEDS  0xFF

MODULE_DESCRIPTION("LED blinking");
MODULE_LICENSE("GPL");

static struct kobject *led_kobj;
static struct timer_list my_timer;
static struct tty_driver *my_driver;
static int _kbledstatus = RESTORE_LEDS;
static int led_mask = 0;
static int timer_active = 0;

static void my_timer_func(struct timer_list *ptr)
{
    int *pstatus = &_kbledstatus;

    if (*pstatus == led_mask)
        *pstatus = RESTORE_LEDS;
    else
        *pstatus = led_mask;

    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);

    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);
}

static ssize_t led_control_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", led_mask);
}

static ssize_t led_control_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int new_mask;

    if (sscanf(buf, "%d", &new_mask) != 1)
        return -EINVAL;

    if (new_mask < 0 || new_mask > 7)
        return -EINVAL;

    if (timer_active) {
        del_timer(&my_timer);
        timer_active = 0;
    }

    led_mask = new_mask;

    if (led_mask == 0) {
        _kbledstatus = RESTORE_LEDS;
        (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, _kbledstatus);
    } else {
        timer_setup(&my_timer, my_timer_func, 0);
        my_timer.expires = jiffies + BLINK_DELAY;
        add_timer(&my_timer);
        timer_active = 1;
    }

    return count;
}

static struct kobj_attribute led_control_attribute =__ATTR(led_control, 0660, led_control_show, led_control_store);

static int __init led_blink_init(void)
{
    int error = 0;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);

    my_driver = vc_cons[fg_console].d->port.tty->driver;

    led_kobj = kobject_create_and_add("systest", kernel_kobj);
    if (!led_kobj)
        return -ENOMEM;

    error = sysfs_create_file(led_kobj, &led_control_attribute.attr);
    if (error) {
        printk(KERN_ERR "failed to create the led_control file in /sys/kernel/systest\n");
        kobject_put(led_kobj);
    }

    return error;
}

static void __exit led_blink_exit(void)
{
    printk(KERN_INFO "kbleds: unloading...\n");
    del_timer(&my_timer);
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);

    kobject_put(led_kobj);
}

module_init(led_blink_init);
module_exit(led_blink_exit);