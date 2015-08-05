#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <fs/proc/internal.h>
#include <linux/sched.h>
#include <asm/uaccess.h> //for copy_from_user
//#include <asm/current.h>
//#include <linux/cred.h>

#define PROCFS_MAX_SIZE         2048 
#define PROCFS_ENTRY_FILENAME   "buffer2k"
static struct proc_dir_entry *Our_Proc_File;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static ssize_t procfs_read(struct file *filp, char *buffer,
                        size_t length, loff_t *offset)
{
    static int finished = 0;
    if(finished)
    {
        printk(KERN_DEBUG "procfs_read: END\n");
        finished = 0;
        return 0;
    }
    finished = 1;
    if(copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
        return -EFAULT;
    printk(KERN_DEBUG "procfs_read: read %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}
static ssize_t procfs_write(struct file *file,  const char *buffer,
                        size_t len, loff_t *off)
{
    if(len>PROCFS_MAX_SIZE)
        procfs_buffer_size = PROCFS_MAX_SIZE;
    else
        procfs_buffer_size = len;
    if(copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
        return -EFAULT;
    printk(KERN_DEBUG "procfs_write: write %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}
//delete the third parameter foo
static int module_permission(struct inode *inode, int op)
{
    //replace "current->euid" with "current_euid()" and also include <linux/sched.h>, see Documentation/credentials.txt
    //replace with 36 for read and 34 for writing, Add ".val" after current_euid()
    if(op==36||(op==34 && (current_euid().val) ==0 ))return 0;
    return -EACCES;
}
int procfs_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}
int procfs_close(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}
static struct file_operations File_Ops_4_Our_Proc_File = {
    .read       = procfs_read,
    .write      = procfs_write,
    .open       = procfs_open,
    .release    = procfs_close,
};
static struct inode_operations Inode_Ops_4_Our_Proc_File = {
    .permission = module_permission,
};
int init_module()
{
//replace proc_create with proc_create
    Our_Proc_File = proc_create(PROCFS_ENTRY_FILENAME, 0644, NULL,NULL);
    if(Our_Proc_File == NULL)
    {
        //replace proc_root with NULL
        //remove_proc_entry(procfs_name, &proc_root);
        remove_proc_entry(PROCFS_ENTRY_FILENAME, NULL);
        printk(KERN_DEBUG "Error: Could not initialize /proc/%s\n", PROCFS_ENTRY_FILENAME);
        return -ENOMEM;
    }
    Our_Proc_File->proc_iops = &Inode_Ops_4_Our_Proc_File;
    Our_Proc_File->proc_fops = &File_Ops_4_Our_Proc_File;
    //This field is deleted in newer version of Kernel.
    //Our_Proc_File->owner     = THIS_MODULE;
    Our_Proc_File->mode      = S_IFREG | S_IRUGO;
    Our_Proc_File->uid       = 0;
    Our_Proc_File->gid       = 0;
    Our_Proc_File->size      = 80;
    printk(KERN_DEBUG "/proc/%s created\n", PROCFS_ENTRY_FILENAME);
    return 0;
}
void cleanup_module()
{
    //replaace proc_root with NULL
    //remove_proc_entry(procfs_name, &proc_root);
    remove_proc_entry(PROCFS_ENTRY_FILENAME, NULL);
    printk(KERN_DEBUG "/proc/%s removed\n", PROCFS_ENTRY_FILENAME);
}
