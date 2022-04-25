#ifndef QLibSane_H
#define QLibSane_H

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#include <QObject>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <QDebug>
#include <QImage>

#ifdef __cplusplus
extern "C" {
#endif

#include "sane/sane.h"
#include "sane/saneopts.h"

#ifdef __cplusplus
}
#endif



class QLibSane : public QObject
{
    Q_OBJECT
public:
    explicit QLibSane(QObject *parent = nullptr);

    static void auth_callback (SANE_String_Const resource,
               SANE_Char * username, SANE_Char * password){}

public slots:
    //初始化sane
    SANE_Status init();
    //获取所有设备
    SANE_Status get_devices(const SANE_Device ***device_list);
    // Open a device
    SANE_Status open_device(SANE_Device *device, SANE_Handle *sane_handle);
    // Cancel scanning
    void cancle_scan(SANE_Handle sane_handle);
    // Close SANE device
    void close_device(SANE_Handle sane_handle);
    // Release SANE resources
    void exit();
    //get_parametersdsr
    SANE_Option_Descriptor* get_parameters_dsr(SANE_Handle handle, SANE_Int option);
    //get_parameters
    void get_paramters(SANE_Handle handle,SANE_Parameters * params);

    void get_option(SANE_Handle handle, SANE_Int option, void* value,SANE_Int* info);

    void set_option(SANE_Handle handle, SANE_Int option, void* value,SANE_Int* info);

    void set_option_auto(SANE_Handle handle, SANE_Int option, void* value,SANE_Int * info);

    SANE_Status StartScan(SANE_Handle sane_handle);

    SANE_Status write_pnm_header (SANE_Frame format, int width, int height, int depth, FILE *ofp);

    SANE_Status SaveScanImage(SANE_Handle device, QString filepath, int flags, int cropout);

};

#endif // QLibSane_H
