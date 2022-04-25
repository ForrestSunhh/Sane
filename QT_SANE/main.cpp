#include "mainwindow.h"
#include <QApplication>
#include <QThread>
#include "QLibSane.h"
#include <QDebug>
#include "sane/sane.h"
#include <QImage>
#include <QMap>

static void auth_callback (SANE_String_Const resource,
           SANE_Char * username, SANE_Char * password){}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if 1

    MainWindow w;
    w.show();
    return a.exec();

#else
    //遍历所有的参数，并打印他们的序号、名字和类型
    QLibSane* scan = new QLibSane();
    SANE_Status status;
    SANE_Handle handle;
    SANE_Device** device_list;
    SANE_Device* device;
    QMap<QString, int> optmap_name;
    QMap<SANE_Value_Type, int> optmap_type;
    QMap<SANE_Unit, int> optmap_unit;


    status = scan->init();


    status = scan->get_devices(&device_list);

    QStringList devs;

    for(int i = 0; device_list[i]!=NULL;i++){
        device = device_list[i];
        qDebug()<<"name:"<<device->name;
        qDebug()<<"vendor:"<<device->vendor;
        qDebug()<<"model:"<<device->model;
        qDebug()<<"type:"<<device->type;
        devs.append(device->name);
    }

    device = device_list[0];
    status = scan->open_device(device,&handle);

    int optnumbers;

    scan->get_option(handle,0,&optnumbers,NULL);
    if(!status)
    {
        for(int i=1; i<optnumbers; i++)
        {
           SANE_Option_Descriptor* option_dsr= scan->get_parameters_dsr(handle, i);
           if(option_dsr->name!=NULL)
           {
               optmap_name.insert(option_dsr->name, i);
               optmap_type.insert(option_dsr->type, i);
               optmap_unit.insert(option_dsr->unit, i);
               qDebug()<<i<<" "<<option_dsr->name<<" "<<option_dsr->type<<" "<<option_dsr->unit;
           }
        }
    }

    return a.exec();
#endif


}
