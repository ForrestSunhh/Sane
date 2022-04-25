#include "QLibSane.h"
#include <unistd.h>
#include "QLibSane.h"
#include <QDebug>

QLibSane::QLibSane(QObject *parent) : QObject(parent)
{

}



#define STRIP_HEIGHT	256
static SANE_Handle device = NULL;
static int verbose;
static SANE_Byte *buffer;
static size_t buffer_size;


/**********************************************
 * @brief   写pnm文件头
 * @param[in] format 扫描模式（黑彩灰）
 * @param[in] width  pnm文件宽度
 * @param[in] height pnm文件高度
 * @param[in] depth  pnm文件位深度
 * @param[in] ofp    pnm文件指针
 * @return int 0:成功 -1:失败
 * @author: SunHaohao
 * @date: 2022/01/14
***********************************************/
SANE_Status QLibSane::write_pnm_header(SANE_Frame format, int width, int height, int depth, FILE *ofp)
{
    int result;
    switch (format)
    {
        case SANE_FRAME_RGB:
            if(depth!=8)
            {
                return SANE_STATUS_INVAL;
            }
            result = fprintf (ofp, "P6\n# SANE data follows\n%d %d\n%d\n", width, height, 255);
            break;
        default:
            if (depth == 1)
            {
                result = fprintf (ofp, "P4\n# SANE data follows\n%d %d\n", width, height);
            }
            else if(depth == 8)
            {
                result = fprintf (ofp, "P5\n# SANE data follows\n%d %d\n%d\n", width, height, 255);
            }
            break;
    }
    if(result>0)
    {
        return SANE_STATUS_GOOD;
    }
    else
    {
        return SANE_STATUS_ACCESS_DENIED;
    }
}

/**********************************************
 * @brief   保存扫描图像
 * @param[in] device 设备指针
 * @param[in] filepath  文件路径
 * @return SANE_Status SANE_Status
 * @author: SunHaohao
 * @date: 2022/01/14
***********************************************/
SANE_Status QLibSane::SaveScanImage(SANE_Handle device, QString filepath, int flags, int cropout)
{
    QString tmppath = filepath.split(".")[0].append(".pnm");
    FILE *ofp = NULL;
    buffer_size = (32 * 1024);
    buffer = static_cast<SANE_Byte*>(malloc (buffer_size));
    int len;
    SANE_Parameters parm;
    SANE_Status status;

    if (NULL == (ofp = fopen (tmppath.toLatin1().data(), "w")))
    {
        status = SANE_STATUS_ACCESS_DENIED;
        goto cleanup;
    }

    status = sane_get_parameters (device, &parm);
    if (status != SANE_STATUS_GOOD)
    {
      goto cleanup;
    }

    status = write_pnm_header(parm.format, parm.pixels_per_line,parm.lines, parm.depth, ofp);

    while (status == 0)
    {
        status = sane_read (device, buffer, buffer_size, &len);
        if (status != SANE_STATUS_GOOD)
        {
            if (status != SANE_STATUS_EOF)
            {
                goto cleanup;
            }
            break;
        }
        fwrite(buffer, 1, len, ofp);
    }
    fflush(ofp);
    switch (status)
    {
        case SANE_STATUS_GOOD:
        case SANE_STATUS_EOF:
             {
                  status = SANE_STATUS_GOOD;
                  if (!ofp || 0 != fclose(ofp))
                  {
                      status = SANE_STATUS_ACCESS_DENIED;
                      break;
                  }
                  else
                  {
                      ofp = NULL;
                      QImage* image = new QImage(tmppath);
                      if(image->depth()==1)
                      {
                          QImage bwimage = image->convertToFormat(QImage::Format_Mono);
                          bwimage.save(filepath);
                      }
                      else if(image->depth() == 8)
                      {
                          QImage grayimage = image->convertToFormat(QImage::Format_Grayscale8);
                          grayimage.save(filepath);
                      }
                      else
                      {
                          QImage colorimage = image->convertToFormat(QImage::Format_RGB888);
                          colorimage.save(filepath);
                      }
                      if(image)
                      {
                          delete image;
                          image = NULL;
                      }
//                      remove(tmppath.toLatin1().data());
                  }
              }
              break;
        default:
              break;
    }

cleanup:

    if (ofp)
    {
        fclose(ofp);
        ofp = NULL;
    }

    if (buffer)
    {
        free (buffer);
        buffer = NULL;
    }
    return status;
}

//sane扫描
SANE_Status QLibSane::StartScan(SANE_Handle sane_handle)
{
    SANE_Status status = sane_start(sane_handle);
    return status;
}

//sane初始化
SANE_Status QLibSane::init()
{
    SANE_Int version_code = 0;
    SANE_Status sane_status = sane_init(&version_code, auth_callback);

    return sane_status;
}

//sane查找设备
SANE_Status QLibSane::get_devices(const SANE_Device ***device_list)
{
    SANE_Status sane_status = sane_get_devices(device_list, SANE_TRUE);//查找设备
    return sane_status;
}

//sane打开设备
SANE_Status QLibSane::open_device(SANE_Device *device, SANE_Handle *sane_handle)
{

    SANE_Status sane_status = sane_open(device->name, sane_handle);
    return sane_status;
}

//sane获取参数描述
 SANE_Option_Descriptor* QLibSane::get_parameters_dsr(SANE_Handle handle, SANE_Int option)
{
    SANE_Option_Descriptor* paramdsr = sane_get_option_descriptor(handle, option);
    return paramdsr;
}

//sane获取扫描属性
void QLibSane::get_paramters(SANE_Handle handle, SANE_Parameters *params)
{
    sane_get_parameters(handle, params);
}

//sane取消扫描
void QLibSane::cancle_scan(SANE_Handle sane_handle)
{
    sane_cancel(sane_handle);
}

//sane关闭设备
void QLibSane::close_device(SANE_Handle sane_handle)
{
    sane_close(sane_handle);
}

//sane释放资源
void QLibSane::exit()
{
    sane_exit();
}

//sane获取参数值
void QLibSane::get_option(SANE_Handle handle, SANE_Int option, void *value, SANE_Int *info)
{
    sane_control_option(handle, option, SANE_ACTION_GET_VALUE, value, info);
}

//sane设置参数值
void QLibSane::set_option(SANE_Handle handle, SANE_Int option, void *value, SANE_Int *info)
{
    sane_control_option(handle, option, SANE_ACTION_SET_VALUE, value, info);
}

//sane设置参数值(自动)
void QLibSane::set_option_auto(SANE_Handle handle, SANE_Int option, void *value, SANE_Int *info)
{
    sane_control_option(handle, option, SANE_ACTION_SET_AUTO, value, info);
}
