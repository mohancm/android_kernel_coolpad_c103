
#ifndef _TYPE_C_INFO_H_
#define _TYPE_C_INFO_H_

enum {
    TYPE_C_DIR_TOP= 0,
    TYPE_C_DIR_BOTTOM = 1,
};

enum {
    TYPE_C_MODE_MAINTAIN= 0,
    TYPE_C_MODE_UFP = 1,
    TYPE_C_MODE_DFP = 2,
    TYPE_C_MODE_DRP = 3,
};


struct typec_info_data {
    u8 typec_dir;
    u8 typec_mode;
    /*LAFITE-395:yuquan added type-c connecting through CC cable begin:*/
    u8 typec_mode_result;
    /*LAFITE-395:yuquan added type-c connecting through CC cable end:*/
    /*LAFITE-1101:yuquan added type-c info notify ready,begin:*/
    u8 typec_notif_ready;
    /*LAFITE-1101:yuquan added type-c info notify node,end:*/
};

#endif /* _TYPE_C_INFO_H_ */
