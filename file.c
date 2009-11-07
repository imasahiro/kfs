#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/namei.h>

static ssize_t default_read_file(struct file *file, char __user *buf,
        size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t default_write_file(struct file *file, const char __user *buf,
        size_t count, loff_t *ppos)
{
    return count;
}

static int default_open(struct inode *inode, struct file *file)
{
    if (inode->i_private)
        file->private_data = inode->i_private;

    return 0;
}

const struct file_operations kfs_file_operations = {
    .read =  default_read_file,
    .write = default_write_file,
    .open =  default_open,
};

static void *debugfs_follow_link(struct dentry *dentry, struct nameidata *nd)
{
    nd_set_link(nd, dentry->d_inode->i_private);
    return NULL;
}

const struct inode_operations kfs_link_operations = {
    .readlink       = generic_readlink,
    .follow_link    = debugfs_follow_link,
};

