#ifndef _LINUX_TYPE_C_NOTIFIER_H
#define _LINUX_TYPE_C_NOTIFIER_H

#include <linux/kgdb.h>


#include <linux/fs.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/list.h>
#include <linux/backlight.h>
#include <linux/slab.h>
#include <asm/io.h>

#define TYPE_C_GET_ORIENTATION   0x0001
#define TYPE_C_SET_MODE                     0x0002
/*LAFITE-1101:yuquan added type-c info notify ready,begin:*/
#define TYPE_C_INFO_NOTIFY_READY       0x0004
/*LAFITE-1101:yuquan added type-c info notify ready,end:*/
/*LAFITE-395:yuquan added type-c connecting through CC cable begin:*/
#define TYPE_C_INFO_MODE_RESULT       0x0005
/*LAFITE-395:yuquan added type-c connecting through CC cable end:*/
/*LAFITE-4113:yuquan added begin:set otg power*/
#define TYPE_C_SET_OTG_POWER       0x0006
/*LAFITE-4113:yuquan added end:set otg power*/
struct type_c_event_data {
    void *data;
};

extern int type_c_get_register_client(struct notifier_block *nb);
extern void type_c_get_unregister_client(struct notifier_block *nb);
extern int type_c_get_notifier_call_chain(unsigned long val,void *v);

extern int type_c_set_register_client(struct notifier_block *nb);
extern void type_c_set_unregister_client(struct notifier_block *nb);
extern int type_c_set_notifier_call_chain(unsigned long val,void *v);
/*LAFITE-4113:yuquan added begin:set otg power*/
extern int type_c_otg_power_register_client(struct notifier_block *nb);
extern void type_c_otg_power_unregister_client(struct notifier_block *nb);
extern int type_c_otg_power_notifier_call_chain(unsigned long val,void *v);
/*LAFITE-4113:yuquan added end:set otg power*/
#endif /* _LINUX_TYPE_C_NOTIFIER_H */
