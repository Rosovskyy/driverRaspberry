#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Markiian Matsiuk <matsiuk@ucu.edu.ua>");
MODULE_DESCRIPTION("A simple Linux driver for 7 segment display");
MODULE_VERSION("0.1");

static unsigned int currentNumber = 0;           ///< Default GPIO for the LED is 49
module_param(currentNumber, uint, 0660);          ///< Param desc. S_IRUGO can be read/not changed
MODULE_PARM_DESC(currentNumber, " Current number on display (default=0)");

//           4
//      +========+
//      |        |
//   27 |        | 17
//      |   22   |
//      +========+
//      |        |
//   26 |        | 6
//      |   13   |
//      +========+  O- 5

int pins[] = {5,22,27,26,13,6,17,4};

char numbers[] = {
        0b1111110,
        0b0110000,
        0b1101101,
        0b1111001,
        0b0110011,
        0b1011011,
        0b1011111,
        0b1110000,
        0b1111111,
        0b1111011
};

char data;

void display_number(char number) {
    for (int i = 0; i < 8; ++i) {
        gpio_set_value(pins[i], number & 1)
        number >>= 1;
    }
}


static ssize_t num_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "Number: %i\n", (unsigned char) currentNumber);
}

static ssize_t num_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
    if (strlen(buf) == 2 && ((buf[0]) >= '0' && (buf[0]) <= '9')) {
        display_number(numbers[buf[0]]);
    }
    return count;
}



static struct kobj_attribute num_attr = __ATTR(currentNumber, 0660, num_show, num_store);

static struct attribute *segment_display_attrs[] = {
        &num_attr.attr,
        NULL,
};

static struct attribute_group attr_group = {
        .name  = "s7_display",
        .attrs = segment_display_attrs,
};

static struct kobject *segment_display_kobj;            


static int __init md_init(void)
{
    int result = 0;
    printk(KERN_INFO "[segment_display] - Initializing segment_display driver\n");

    segment_display_kobj = kobject_create_and_add("segment_display", kernel_kobj->parent);
    if(!segment_display_kobj){
        printk(KERN_ALERT "[segment_display] - failed to create kobject\n");
        return -ENOMEM;
    }

    result = sysfs_create_group(segment_display_kobj, &attr_group);
    if(result) {
        printk(KERN_ALERT "[segment_display] - failed to create sysfs group\n");
        kobject_put(segment_display_kobj);         
        return result;
    }

    gpio_set_value(numbers[0], 0);


    return 0;
}

static void __exit md_exit(void)
{
    kobject_put(segment_display_kobj);
    display_number(0b0000000);
    printk("segment_display driver unloaded!\n");
}

module_init(md_init);
module_exit(md_exit);