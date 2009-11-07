/*
 * Script file system.
 * Copyright (c) 2009 Masahiro Ide
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#define printf(fmt, ...) \
    printk(KERN_INFO "KFS: %s() at %d : " fmt, __FUNCTION__, __LINE__,## __VA_ARGS__)
#define KFS_MAGIC 0x6b6673 /* "kfs" */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("masahiro ide");


struct kfs_root {
    struct super_block *sb;
};

static int kfs_fill_super(struct super_block *sb, void *data, int silent)
{
    int rc;
    static struct tree_descr kfs_files[] = {{""}};

    rc = simple_fill_super(sb, KFS_MAGIC, kfs_files);
    if (rc != 0) {
        printk(KERN_ERR "%s failed %d while creating inodes\n",
                __func__, rc);
        return rc;
    }
    return 0;
}

static int kfs_get_sb(struct file_system_type *fs_type,
        int flags,  const char *dev_name,
        void *data, struct vfsmount *mnt)
{
    printf("dev_name='%s',data='%s'",dev_name,(char*)data);
    return get_sb_nodev(fs_type, flags, data, kfs_fill_super, mnt);
}

static void kfs_kill_sb(struct super_block *sb) {
    kfree(sb->s_fs_info);
    kill_litter_super(sb);
}

static struct file_system_type kfs_type = {
    .name = "kfs",
    .get_sb  = kfs_get_sb,
    .kill_sb = kfs_kill_sb,
    .owner = THIS_MODULE
};

static int __init kfs_module_init(void)
{
    printf("");
    register_filesystem(&kfs_type);
    return 0;
}

static void __exit kfs_module_exit(void)
{
    printf("");
    unregister_filesystem(&kfs_type);
}

module_init(kfs_module_init);
module_exit(kfs_module_exit);
