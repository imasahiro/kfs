
static int knh_device_open (struct inode *inode , struct file *filp);
static ssize_t knh_device_read(struct file *filp, char __user *user_buf,
        size_t count, loff_t *offset);
static ssize_t knh_dev_write(struct file *file,const char __user *buf,
        size_t count,loff_t *offp) ;

static struct file_operations knh_fops = {
    .owner = THIS_MODULE,
    .open  = knh_device_open,
    .read  = knh_device_read,
    .ioctl = knh_dev_ioctl,
};

struct knh_file_operations {
    // loff_t (*llseek) (struct file *, loff_t, int);
    knh_Closure_t *llseek;
    // ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    knh_Closure_t *read;
    //ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    knh_Closure_t *write;
    //ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
    knh_Closure_t *aio_read;
    //ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
    knh_Closure_t *aio_write;
    //int (*readdir) (struct file *, void *, filldir_t);
    knh_Closure_t *readdir;
    //unsigned int (*poll) (struct file *, struct poll_table_struct *);
    knh_Closure_t *poll;
    //int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
    knh_Closure_t *ioctl;
    //long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    knh_Closure_t *unlocked_ioctl;
    //long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
    knh_Closure_t *compat_ioctl;
    //int (*mmap) (struct file *, struct vm_area_struct *);
    knh_Closure_t *mmap;
    //int (*open) (struct inode *, struct file *);
    knh_Closure_t *open;
    //int (*flush) (struct file *, fl_owner_t id);
    knh_Closure_t *flush;
    //int (*release) (struct inode *, struct file *);
    knh_Closure_t *release;
    //int (*fsync) (struct file *, struct dentry *, int datasync);
    knh_Closure_t *fsync;
    //int (*aio_fsync) (struct kiocb *, int datasync);
    knh_Closure_t *aio_fsync;
    //int (*fasync) (int, struct file *, int);
    knh_Closure_t *fasync;
    //int (*lock) (struct file *, int, struct file_lock *);
    knh_Closure_t *lock;
    //ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
    knh_Closure_t *sendpage;
    //unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
    knh_Closure_t *get_unmapped_area;
    //int (*check_flags)(int);
    knh_Closure_t *check_flags;
    //int (*flock) (struct file *, int, struct file_lock *);
    knh_Closure_t *flock;
    //ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
    knh_Closure_t *splice_write;
    //ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
    knh_Closure_t *splice_read;
    //int (*setlease)(struct file *, long, struct file_lock **);
    knh_Closure_t *setlease;
};


typedef struct knh_device_t {
    dev_t id;
    char *name;
    struct cdev cdev;
    struct knh_file_operations *op;
} knh_device_t;

typedef struct knh_Chardev_t {
    knh_hObject_t h;
    knh_device_t *device;
} knh_Chardev_t;


