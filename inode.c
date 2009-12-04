#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/namei.h>
#include <linux/fsnotify.h>
#include <linux/string.h>
#include <linux/magic.h>

extern const struct file_operations kfs_file_operations;
extern const struct inode_operations kfs_link_operations;

extern const struct file_system_type kfs_type;

static struct inode *kfs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
    struct inode *inode = new_inode(sb);

    if (inode) {
        inode->i_mode = mode;
        inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
        switch (mode & S_IFMT) {
            default:
                init_special_inode(inode, mode, dev);
                break;
            case S_IFREG:
                inode->i_fop = &kfs_file_operations;
                break;
            case S_IFLNK:
                inode->i_op = &kfs_link_operations;
                break;
            case S_IFDIR:
                inode->i_op = &simple_dir_inode_operations;
                inode->i_fop = &simple_dir_operations;
                /* directory inodes start off with i_nlink == 2
                 * (for "." entry) */
                inc_nlink(inode);
                break;
        }
    }
    return inode; 
}

/* SMP-safe */
static int kfs_mknod(struct inode *dir, struct dentry *dentry,
        int mode, dev_t dev)
{
    struct inode *inode;
    int error = -EPERM;

    if (dentry->d_inode)
        return -EEXIST;

    inode = kfs_get_inode(dir->i_sb, mode, dev);
    if (inode) {
        d_instantiate(dentry, inode);
        dget(dentry);
        error = 0;
    }
    return error;
}

static int kfs_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
    int res;

    mode = (mode & (S_IRWXUGO | S_ISVTX)) | S_IFDIR;
    res = kfs_mknod(dir, dentry, mode, 0);
    if (!res) {
        inc_nlink(dir);
        fsnotify_mkdir(dir, dentry);
    }
    return res;
}

static int kfs_link(struct inode *dir, struct dentry *dentry, int mode)
{
    mode = (mode & S_IALLUGO) | S_IFLNK;
    return kfs_mknod(dir, dentry, mode, 0);
}

static int kfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
    int res;

    mode = (mode & S_IALLUGO) | S_IFREG;
    res = kfs_mknod(dir, dentry, mode, 0);
    if (!res)
        fsnotify_create(dir, dentry);
    return res;
}

static inline int kfs_positive(struct dentry *dentry)
{
    return dentry->d_inode && !d_unhashed(dentry);
}

static int kfs_create_by_name(const char *name, mode_t mode,
        struct dentry *parent,
        struct dentry **dentry)
{
    int error = 0;

    /* If the parent is not specified, we create it in the root.
     * We need the root dentry to do this, which is in the super 
     * block. A pointer to that is in the struct vfsmount that we
     * have around.
     */
    if (!parent) {
        /*
           if (debugfs_mount && debugfs_mount->mnt_sb) {
           parent = debugfs_mount->mnt_sb->s_root;
           }
         */
    }
    if (!parent) {
        pr_debug("debugfs: Ah! can not find a parent!\n");
        return -EFAULT;
    }

    *dentry = NULL;
    mutex_lock(&parent->d_inode->i_mutex);
    *dentry = lookup_one_len(name, parent, strlen(name));
    if (!IS_ERR(*dentry)) {
        switch (mode & S_IFMT) {
            case S_IFDIR:
                error = kfs_mkdir(parent->d_inode, *dentry, mode);
                break;
            case S_IFLNK:
                error = kfs_link(parent->d_inode, *dentry, mode);
                break;
            default:
                error = kfs_create(parent->d_inode, *dentry, mode);
                break;
        }
        dput(*dentry);
    } else
        error = PTR_ERR(*dentry);
    mutex_unlock(&parent->d_inode->i_mutex);

    return error;
}

/**
 * debugfs_create_file - create a file in the debugfs filesystem
 * @name: a pointer to a string containing the name of the file to create.
 * @mode: the permission that the file should have
 * @parent: a pointer to the parent dentry for this file.  This should be a
 *          directory dentry if set.  If this paramater is NULL, then the
 *          file will be created in the root of the debugfs filesystem.
 * @data: a pointer to something that the caller will want to get to later
 *        on.  The inode.i_private pointer will point to this value on
 *        the open() call.
 * @fops: a pointer to a struct file_operations that should be used for
 *        this file.
 *
 * This is the basic "create a file" function for debugfs.  It allows for a
 * wide range of flexibility in createing a file, or a directory (if you
 * want to create a directory, the debugfs_create_dir() function is
 * recommended to be used instead.)
 *
 * This function will return a pointer to a dentry if it succeeds.  This
 * pointer must be passed to the debugfs_remove() function when the file is
 * to be removed (no automatic cleanup happens if your module is unloaded,
 * you are responsible here.)  If an error occurs, %NULL will be returned.
 *
 * If debugfs is not enabled in the kernel, the value -%ENODEV will be
 * returned.
 */
struct dentry *kfs_create_file(const char *name, mode_t mode,
        struct dentry *parent, void *data,
        const struct file_operations *fops)
{
    struct dentry *dentry = NULL;
    int error;
    int kfs_mount_count = 0;
    pr_debug("debugfs: creating file '%s'\n",name);
    
    /*
    error = simple_pin_fs(&kfs_type, &kfs_mount, &kfs_mount_count);
    if (error)
        goto exit;

    error = kfs_create_by_name(name, mode, parent, &dentry);
    if (error) {
        dentry = NULL;
        simple_release_fs(&kfs_mount, &kfs_mount_count);
        goto exit;
    }

    if (dentry->d_inode) {
        if (data)
            dentry->d_inode->i_private = data;
        if (fops)
            dentry->d_inode->i_fop = fops;
    }
    */
exit:
    return dentry;
}

/**
 * debugfs_create_dir - create a directory in the debugfs filesystem
 * @name: a pointer to a string containing the name of the directory to
 *        create.
 * @parent: a pointer to the parent dentry for this file.  This should be a
 *          directory dentry if set.  If this paramater is NULL, then the
 *          directory will be created in the root of the debugfs filesystem.
 *
 * This function creates a directory in debugfs with the given name.
 *
 * This function will return a pointer to a dentry if it succeeds.  This
 * pointer must be passed to the debugfs_remove() function when the file is
 * to be removed (no automatic cleanup happens if your module is unloaded,
 * you are responsible here.)  If an error occurs, %NULL will be returned.
 *
 * If debugfs is not enabled in the kernel, the value -%ENODEV will be
 * returned.
 */
struct dentry *kfs_create_dir(const char *name, struct dentry *parent)
{
    return kfs_create_file(name, 
            S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
            parent, NULL, NULL);
}

static void __kfs_remove(struct dentry *dentry, struct dentry *parent)
{
    int ret = 0;

    if (kfs_positive(dentry)) {
        if (dentry->d_inode) {
            dget(dentry);
            switch (dentry->d_inode->i_mode & S_IFMT) {
                case S_IFDIR:
                    ret = simple_rmdir(parent->d_inode, dentry);
                    break;
                case S_IFLNK:
                    kfree(dentry->d_inode->i_private);
                    /* fall through */
                default:
                    simple_unlink(parent->d_inode, dentry);
                    break;
            }
            if (!ret)
                d_delete(dentry);
            dput(dentry);
        }
    }
}


