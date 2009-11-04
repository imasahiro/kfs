/*
 *  Konoha Chardevice Script
 *
 */

#include <konoha.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define printf(fmt, ...) \
    printk(KERN_INFO "KFS: %s() at %d : " fmt, __FUNCTION__, __LINE__,## __VA_ARGS__)

/* ======================================================================== */
/* [constructors] */

KNHAPI(Chardev*) new_Chardev(Ctx *ctx)
{
    Chardev *cdev = (Chardev*)new_Object_bcid(ctx, CLASS_Chardev, 0);
    cdev->device = KNH_MALLOC(ctx, sizeof(knh_device_t));
    printf("%s\n",__FUNCTION__);
    return cdev;
}

/* ------------------------------------------------------------------------ */

static void knh_Chardev_setName(Ctx *ctx, Chardev *o, char* devname)
{
    char* name = o->device->name;
    name = (char *)KNH_MALLOC(ctx, strlen(devname));
    strncpy(name, devname, sizeof(devname));
    o->device->name = name;
    //fprintf(stderr, "%s,%s,%s\n",__FUNCTION__,devname,o->device->name);
}

/* ------------------------------------------------------------------------ */
/* [new] */
/* @method[VIRTUAL] This! Chardev.new(String! devname) */

METHOD Chardev_new(Ctx *ctx, knh_sfp_t *sfp)
{
    Chardev *o = (Chardev *) sfp[0].o;
    char* devname = knh_String_tochar((String*)sfp[1].o);
    //fprintf(stderr, "%s,[%s]\n",__FUNCTION__,devname);
    knh_Chardev_setName(ctx, o, devname);
    KNH_RETURN(ctx, sfp, o);
}

/* ------------------------------------------------------------------------ */
static int knh_device_open (struct inode* inode, struct file *filp)
{
    filp->private_data = container_of(inode->i_cdev, knh_device_t,cdev);
    printk("%s at %d\n",__FUNCTION__,__LINE__);
    return 0;
}

static int knh_device_release (struct inode* inode, struct file *filp)
{
    knh_device_t *dev = filp->private_data;
    //knh_Chardev_release(ctx, sfp);
    return 0;
}


static ssize_t knh_device_read (struct file* filp, char __user *user_buf,
        size_t count, loff_t *offset)
{
    knh_device_t *dev = filp->private_data;

    Ctx *lctx = knh_getCurrentContext();
    knh_sfp_t *lsfp = KNH_LOCAL(lctx);
    Closure *cc = knh_DictMap_get__b(lctx, dict, B("read"));

    if (cc) {
        knh_Closure_invokesfp(lctx, cc, lsfp, 0);
        knh_bytes_t ret = knh_String_tobytes((String*)lsfp[0].o);
        printk("%s at %d [ret=%s]\n",__FUNCTION__,__LINE__,ret.buf);
        if(copy_to_user(user_buf,ret.buf,ret.len)){
            printk(KERN_ALERT "%s: copy_to_user failed\n",dev->name);
            return -EFAULT;
        }
        *offset += ret.len;
        return ret.len;
    }
    return 0;
}

static int knh_dev_ioctl (struct inode *inode, struct file *filp, 
        unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    knh_device_t *dev = filp->private_data;

    switch (cmd) {
        /*
        case FIBDEV_IOC_RESET:
            dev->n = 1;
            break;
        case FIBDEV_IOC_SETN:
            retval = get_user(dev->n, (int __user *)arg);
            if (dev->n > MAXFIBS)
                dev->n = 1;
            break;
        case FIBDEV_IOC_GETN:
            retval = put_user(dev->n, (int __user *)arg);
            break;
            */
        default:
            retval = -ENOTTY;
            break;
    }

    return retval;
}

knh_bool_t knh_Chardev_regist(Ctx *ctx, Chardev *o)
{
    fprintf(stderr, "%s\n",__FUNCTION__);
    knh_device_t *dev = (knh_device_t *) o->device;
    char* name = dev->name;
    int err = alloc_chrdev_region(&dev->id, 0, 1, name);
    if(err){
        printk(KERN_ALERT "%s: alloc_chrdev_region() failed (%d)\n",name,err);
        return 0; // false
    }
    cdev_init(&dev->cdev,&knh_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, dev->id, 1);
    if(err){
        printk(KERN_ALERT "%s: cdev_add() failed (%d)\n",name,err);
        return 0; // false
    }
    o->isEnable = 1;

    return 1;
}

/* ------------------------------------------------------------------------ */
/* @method Boolean! Chardev.regist() */

METHOD Chardev_regist(Ctx *ctx, knh_sfp_t *sfp)
{
    Chardev *cdev = (Chardev *) sfp[0].o;
    cdev->device->ctx = (Context *)ctx;
    KNH_RETURN_Boolean(ctx, sfp, knh_Chardev_regist(ctx, cdev))
}

/* ------------------------------------------------------------------------ */
/* @method Boolean! Chardev.unregist() */

METHOD Chardev_unregist(Ctx *ctx, knh_sfp_t *sfp)
{
    Chardev *cdev = (Chardev *) sfp[0].o;
    knh_device_t *device = cdev->device;
    if(cdev->isEnable) {
        cdev_del(&device->cdev);
        unregister_chrdev_region(device->id,1);
        cdev->isEnable = 0;
    }
    KNH_RETURN_Boolean(ctx, sfp, 1);
}

/* ------------------------------------------------------------------------ */
/* @method Boolean! Chardev.addFunc(String op, Closure c) */

METHOD Chardev_addFunc(Ctx *ctx, knh_sfp_t *sfp)
{
    Chardev *cdev  = (Chardev*) sfp[0].o;
    String* opname = (String*)  sfp[1].o;
    Closure *cc    = sfp[2].cc;

    DictMap *fmap = cdev->device->fmap;
    knh_DictMap_set(ctx, fmap, opname, (Object *)cc);
    KNH_RETURN_Boolean(ctx, sfp, 1);
}


/* ------------------------------------------------------------------------ */
static int __init kcs_module_init(void)
{
    printf("");
    return 0;
}

static void __exit kcs_module_exit(void)
{
    printf("");
}

module_init(kcs_module_init);
module_exit(kcs_module_exit);
