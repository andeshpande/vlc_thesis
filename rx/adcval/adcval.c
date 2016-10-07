#include <linux/sched.h>
#include <linux/interrupt.h> /* mark_bh */
#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/skbuff.h>
#include <linux/in6.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/unistd.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/iio/consumer.h>
#include <linux/iio/iio.h>
#include <asm/checksum.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/wait.h>
#include <linux/timex.h>
#include <linux/clk.h>
#include <linux/pinctrl/consumer.h>
#include <linux/proc_fs.h>
#include <linux/ktime.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "adcval.h"

MODULE_AUTHOR("Aniruddha");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module to retrieve LED/PD values from MSP3008 ADC");

#define MAX_SAMPLES 100
// Global Variables and structs
volatile void* gpio1;
volatile void* gpio2;
static int adc_ch = 0;
static int bit_clc = 1<<BIT_CLC;
struct proc_dir_entry *adcval_dir, *lval_file, *pval_file;

// Function prototypes
// static void get_adcval(void);
static int  SPI_read_from_adc(void);
static void gpio_init(void);
static void switch_led_to_rx(void);
// static int thread_function(void * data);

// struct task_struct *task;
int ret;
void *data = NULL;
int flag = 0;

//------------------------------------------------------------------------------
int proc_read_pval(char *page, char **start, off_t off, int count, int *eof,
  void *data)
{
  int pval;
  memset(page, '\0', 4);
  
  adc_ch = 1;
  pval = SPI_read_from_adc();
  count = sprintf(page, "%d", pval);

  return count;
}
//------------------------------------------------------------------------------
int proc_read_lval(char *page, char **start, off_t off, int count, int *eof,
  void *data)
{
  int lval;
  memset(page, '\0', 4);

  adc_ch = 0;
  lval =  SPI_read_from_adc();
  count = sprintf(page, "%d", lval);

  return count;
}
//------------------------------------------------------------------------------
// Module init
static int __init adcval_init(void) {
	printk("ADCVAL: Module Loaded\n");

  adcval_dir = proc_mkdir("adcval", NULL);
  //val_file   = create_proc_entry("val", 0444, adcval_dir);

  lval_file  = create_proc_entry("lval",0444, adcval_dir);
  pval_file  = create_proc_entry("pval",0444, adcval_dir);

  gpio_init();
  switch_led_to_rx();

  lval_file->read_proc = proc_read_lval;
  pval_file->read_proc = proc_read_pval;
  // task = kthread_run(&thread_function, data ,"adcval");

  // printk(KERN_INFO"ADCVAL: Kernel Thread : %s\n", task->comm);

  return 0;
}

static void switch_led_to_rx(void)
{
  gpio_set_value(GPIO_LED_ANODE, GPIOF_INIT_LOW);
  gpio_set_value(GPIO_BUFFER_CONTROL, GPIOF_INIT_HIGH);
}

//------------------------------------------------------------------------------
// Init for GPIOs
static void gpio_init(void) {

  if ( gpio_request(GPIO_LED_ANODE, "LED_ANODE")
    || gpio_request(GPIO_LED_CATHODE, "LED_CATHODE")
    || gpio_request(GPIO_BUFFER_CONTROL, "BUFFER_CONTROL")
  //|| gpio_request(GPIO_H_POWER_LED, "H_POWER_LED")
    || gpio_request(GPIO_LED_OR_PD, "LED_OR_PD") ) 

  {
    printk("ADCVAL: GPIO - Request GPIO failed!\n");	
  }

    // GPIOs
  gpio_direction_output(GPIO_LED_ANODE, GPIOF_INIT_LOW);
  gpio_direction_output(GPIO_LED_CATHODE, GPIOF_INIT_LOW);
  gpio_direction_output(GPIO_BUFFER_CONTROL, GPIOF_INIT_HIGH);
    //gpio_direction_output(GPIO_H_POWER_LED, GPIOF_INIT_LOW);
  gpio_direction_output(GPIO_LED_OR_PD, GPIOF_INIT_LOW);
    // GPIOs for SPI
  
  if ( gpio_request(SPI_CLC, "SPI_CLC")
    || gpio_request(SPI_MISO, "SPI_MISO")
    || gpio_request(SPI_MOSI, "SPI_MOSI")
    || gpio_request(SPI_CS, "SPI_CS") )
  {
    printk("ADCVAL: SPI - Request GPIO failed!\n");
  }

  gpio_direction_output(SPI_CLC, GPIOF_INIT_LOW);
  gpio_direction_input(SPI_MISO);
  gpio_direction_output(SPI_MOSI, GPIOF_INIT_LOW);
  gpio_direction_output(SPI_CS, GPIOF_INIT_LOW);

  gpio1 = ioremap(ADDR_BASE_0, 4);
  gpio2 = ioremap(ADDR_BASE_1, 4);

  printk("ADCVAL: GPIO - Access address to device is:0x%x  0x%x\n",
    (unsigned int) gpio1, (unsigned int) gpio2);
}
//------------------------------------------------------------------------------
// static int thread_function(void * data) {
//   //  ktime_t start, end;
//   //  int diff;
//   //  int period = 20;
//   //  int delta  = 2;

//    while(1)
//    {
//       // start = ktime_get();
//       get_adcval();
//       //  end  = ktime_get();
//       //  diff = ktime_to_ns(ktime_sub(end, start));
//       //  diff = diff / 1000;
//       //  printk("ADCVAL -- %u\n", (unsigned int)diff);
//       //  usleep_range(period - delta - diff, period + delta - diff);

//       usleep_range(5, 10);
//      if (kthread_should_stop()) return 0;
//    }

//    return 0;
//  }

//------------------------------------------------------------------------------
// static void get_adcval(void) {
//   int pval, lval;
// 	adc_ch = 0;
//   lval   = SPI_read_from_adc();

// 	adc_ch = 1;
// 	pval   = SPI_read_from_adc();

//   lval_file->data      = &lval;
//   pval_file->data      = &pval;
//   lval_file->read_proc = proc_read;
//   pval_file->read_proc = proc_read;

//   //snprintf(val, 10, "%d %d", lval, pval);
// //  printk("val:%s\n", val);
//   //val_file->data       = val;
//   //val_file->read_proc  = proc_read;
// }

//--------------------------------------------------------------------
#define SPI_DELAY_CNT 10
// Delay in terms of count
static void inline delay_n_NOP(void)
{
  int i;
  for(i=SPI_DELAY_CNT; i>0; i--)
    ;
}
//--------------------------------------------------------------------
static void SPI_write_sfd_and_ch(void)
{
    //unsigned char write_byte = 0x18; // 0001 1000  channel 0 of the ADC
    unsigned char write_byte = 0x18 + adc_ch; // 0001 1000  channel 0 of the AD
    unsigned char shift = 0x10; // 0001 1000
    while (shift > 0) {
      writel(bit_clc, gpio2+CLEAR_OFFSET);
      delay_n_NOP();
      if ((_Bool) (write_byte & shift)) {
        writel(1<<BIT_MOSI, gpio2+SET_OFFSET);
        delay_n_NOP();
      } else {
        writel(1<<BIT_MOSI, gpio2+CLEAR_OFFSET);
        delay_n_NOP();
      }
      shift >>= 1;
      writel(bit_clc, gpio2+SET_OFFSET);
    }
  }
//--------------------------------------------------------------------
  static int SPI_read_from_adc(void)
  {
    unsigned int value = 0, index;

    writel(1<<BIT_CS, gpio1+CLEAR_OFFSET);
    delay_n_NOP();
    SPI_write_sfd_and_ch();
    // Skip the first interval
    writel(bit_clc, gpio2+CLEAR_OFFSET);
    delay_n_NOP();
    writel(bit_clc, gpio2+SET_OFFSET);
    delay_n_NOP();
    // Read the value
    for (index=0; index<11; index++)
    {
      writel(bit_clc, gpio2+CLEAR_OFFSET);
      delay_n_NOP();
      value <<= 1;
      value |= (0x1 & (readl(gpio1+READ_OFFSET)>>BIT_MISO));
      writel(bit_clc, gpio2+SET_OFFSET);
      delay_n_NOP();
    }
    writel(bit_clc, gpio2+CLEAR_OFFSET);
    delay_n_NOP();
    writel(1<<BIT_CS, gpio1+SET_OFFSET);
    delay_n_NOP();

    return value;
  }
//--------------------------------------------------------------------
// Module exit
  static void __exit adcval_exit(void) {
	// kthread_stop(task);

   iounmap(gpio1);
   iounmap(gpio2);

	// Clean the GPIOs
   gpio_free(GPIO_LED_ANODE);
   gpio_free(GPIO_LED_CATHODE);
   gpio_free(GPIO_BUFFER_CONTROL);
//	gpio_free(GPIO_H_POWER_LED);
   gpio_free(GPIO_LED_OR_PD);

   gpio_free(SPI_CLC);
   gpio_free(SPI_MISO);
   gpio_free(SPI_MOSI);
   gpio_free(SPI_CS);

   remove_proc_entry("lval", adcval_dir);
   remove_proc_entry("pval", adcval_dir);
  //remove_proc_entry("val", adcval_dir);
   remove_proc_entry("adcval", NULL);

   printk("ADCVAL: Cleanup.\n");
 }

 module_init(adcval_init);
 module_exit(adcval_exit);
