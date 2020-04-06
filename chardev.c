/*
 * chardev.c: Crea un chardev que se le envia datos y se los puede leer luego. 
 *
 * Basado en chardev.c desde TLDP.org's LKMPG book.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> /* for put_user */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define DRIVER_AUTHOR "Andres Blas Pujadas"
#define DRIVER_DESC "Un driver para un dispositivo de tipo char."
#define DRIVER_LICENSE "GPL"

#define SUCCESS 0
#define DEVICE_NAME "UNGS"
#define BUF_LEN 80

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("0.1");
// MODULE_SUPPORTED_DEVICE(DEVICE_NAME);


/*
 * Global variables are declared as static, so are global within the file.
 */

static int Major;
static int Device_Open = 0;
static char msg[BUF_LEN];
static int msg_length = 0;

static struct file_operations fops = {
	.owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/*
 * This function is called when the module is loaded
 */
static int __init chardev_init(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
        printk(KERN_ALERT "Registrando char device fallo con %d\n", Major);
        return Major;
    }

    // int mknodResult = mknod(strcat("/dev/", DEVICE_NAME), S_IFCHR | 0666, )

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
static void __exit chardev_exit(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_INFO "Descargando modulo... bye bye.\n");
}

module_init(chardev_init);
module_exit(chardev_exit);

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev
 */
static int device_open(struct inode *inode, struct file *filp)
{
    if (Device_Open)
        return -EBUSY;
    
    Device_Open++;
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "DEVICE_OPEN called.");
    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *filp)
{
    Device_Open--;
    module_put(THIS_MODULE);
    printk(KERN_INFO "DEVICE_RELEASE called.");
    return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to read
 * from it.
 */
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */
                           char *buffer,      /* buffer to fill with data */
                           size_t length,     /* length of the buffer     */
                           loff_t *offset)
{
   int error_count = 0;
   printk(KERN_INFO "Msg now has %s", msg);
   error_count = copy_to_user(buffer, msg, msg_length);
   int oldMsgLength = msg_length;
   msg_length = 0;
   return (oldMsgLength);
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/UNGS
 */
static ssize_t device_write(struct file *filp, const char *tmp, size_t length, loff_t *offset) {
    memset(msg, 0, sizeof msg);
    copy_from_user(msg, tmp, length);
    msg_length = strlen(msg);
    printk(KERN_INFO "Msg now has %s", msg);
    return length;
}