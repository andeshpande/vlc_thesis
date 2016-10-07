#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <asm/io.h>		// ioremap
#include <linux/pinctrl/consumer.h>
#include <linux/delay.h>

#include <linux/types.h>
#include <linux/errno.h>
#include "srclight.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anirudha Deshpande");
MODULE_DESCRIPTION("Module to switch in hpl");

volatile void* gpio1;
volatile void* gpio2;

static int __init srclight_init(void) {
	
	int c = 100;
	printk(KERN_INFO"SRCLIGHT: Initialized.\n");
	gpio_request(GPIO_LED_ANODE, "LED_ANODE");
	gpio_request(GPIO_LED_CATHODE, "LED_CATHODE");

   	gpio_direction_output(GPIO_LED_ANODE, GPIOF_INIT_HIGH);
   	gpio_direction_output(GPIO_LED_CATHODE, GPIOF_INIT_LOW);

    	
	gpio1 = ioremap(ADDR_BASE_0, 4);
   	gpio2 = ioremap(ADDR_BASE_1, 4);

	// while (c--)
	// {
	// 	gpio_set_value(GPIO_LED_ANODE, GPIOF_INIT_LOW);
 //       	msleep(100);
	// 	gpio_set_value(GPIO_LED_ANODE, GPIOF_INIT_HIGH);
	// }
	// gpio_request(GPIO_H_POWER_LED, "H_POWER_LED"); 
 //    	gpio_direction_output(GPIO_H_POWER_LED, GPIOF_INIT_HIGH);
	return 0;
}

static void __exit srclight_exit(void) {
	// gpio_free(GPIO_H_POWER_LED);
	gpio_free(GPIO_LED_ANODE);
	gpio_free(GPIO_LED_CATHODE);
	printk(KERN_INFO"SRCLIGHT: Cleanup.\n");
}

module_init(srclight_init);
module_exit(srclight_exit);
