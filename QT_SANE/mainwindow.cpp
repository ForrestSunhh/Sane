#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>

#include <QApplication>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPainter>
#include <QFontDatabase>
#include <QDebug>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    sane = new QLibSane();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    if(sane)
    {
        delete sane;
        sane = NULL;
    }
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //1.初始化sane
    sane_status = sane->init();
    qDebug()<<"初始化设备结果："<<sane_status;
}

void MainWindow::on_pushButton_2_clicked()
{
    if(sane_status!=SANE_STATUS_GOOD)
    {
        return;
    }
    //2.查找设备
    sane_status = sane->get_devices(&device_list);
    qDebug()<<"查找设备结果："<<sane_status;
    device =(*device_list);
    if (!device)
    {
        qDebug()<<"No device connected_ui";
        return ;
    }
    else{
        for(int i = 0; device_list[i]!=NULL;i++){
            qDebug()<<device_list[i];
            device = device_list[i];
            qDebug()<<"name:"<<device->name;
            qDebug()<<"vendor:"<<device->vendor;
            qDebug()<<"model:"<<device->model;
            qDebug()<<"type:"<<device->type;
        }
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    // 3.打开设备，默认打开找到的最后一个扫描仪
    if (!device)
    {
        sane_status = SANE_STATUS_INVAL;
        qDebug()<<"No device connected_ui";
        return ;
    }

    qDebug()<<"name:"<<device->name;
    qDebug()<<"vendor:"<<device->vendor;
    qDebug()<<"model:"<<device->model;
    qDebug()<<"type:"<<device->type;

    sane_status = sane->open_device(device, &sane_handle);

    //维护一个参数序号和参数类型的QMap,获取该扫描仪所有的参数,把名字和类型及对应的参数序号存入qmap
    int optnumbers;
    SANE_Option_Descriptor* option_dsr = NULL;
    if( !sane_status && sane_handle != NULL)
    {
        sane_status = sane_control_option(sane_handle,0,SANE_ACTION_GET_VALUE,&optnumbers,NULL);
    }

    if( !sane_status && sane_handle != NULL)
    {
        for( int i=1; i<optnumbers; i++)
        {
           option_dsr = sane_get_option_descriptor(sane_handle, i);
           if( option_dsr->name != NULL )
           {
               m_optmap_name.insert(i,option_dsr->name);
               m_optmap_type.insert(i, option_dsr->type);
           }
        }
     }
    qDebug()<<"打开设备结果："<<sane_status;
}


void MainWindow::on_pushButton_4_clicked()
{
    if(sane_status!=SANE_STATUS_GOOD)
    {
        return;
    }
    //4.设置参数
    SANE_Int info;
    char valuestr[128];
    int valueint = 0;
    int param = ui->lineEdit->text().toInt();
    if(!m_optmap_type.contains(param))
    {
        qDebug()<<"请检查参数序号是否正确。";
        return;
    }

    switch (m_optmap_type.value(param)) {
    case 0:
    case 1:
        valueint = ui->lineEdit_2->text().toInt();
        sane->set_option(sane_handle,param,&valueint,&info);
        qDebug()<<"要设置的值："<<ui->lineEdit_2->text().toInt()<<"设置成功的值："<<valueint;
        break;
    case 2:
        valueint = SANE_FIX(ui->lineEdit_2->text().toInt());
        sane->set_option(sane_handle,param,&valueint,&info);
        qDebug()<<"要设置的值："<<ui->lineEdit_2->text().toInt()<<"设置成功的值："<<valueint;
        break;
    case 3:
        strcpy(valuestr,ui->lineEdit_2->text().toUtf8().data());
        sane->set_option(sane_handle,param,valuestr,&info);
        qDebug()<<"要设置的值："<<ui->lineEdit_2->text().toUtf8().data()<<"设置成功的值："<<valuestr;
    default:
        break;
    }
}

void MainWindow::on_pushButton_10_clicked()
{
    if(sane_status!=SANE_STATUS_GOOD)
    {
        return;
    }
    //获取参数值
    SANE_Int info;
    char valuestr[128];
    int valueint = 0;
    int param = ui->lineEdit->text().toInt();
    if(!m_optmap_type.contains(param))
    {
        qDebug()<<"请检查参数序号是否正确。";
        return;
    }

    switch (m_optmap_type.value(param)) {
    case 0:
    case 1:
    case 2:
        sane->get_option(sane_handle,param,&valueint,&info);
        qDebug()<<m_optmap_name.value(param)+"参数值int："<<valueint;
        ui->lineEdit_2->setText(QString::number(valueint));
        break;
    case 3:
        sane->get_option(sane_handle,param,valuestr,&info);
        qDebug()<<m_optmap_name.value(param)+"参数值str："<<valuestr;
        ui->lineEdit_2->setText(valuestr);
    default:
        break;
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    if(sane_status!=SANE_STATUS_GOOD)
    {
        return;
    }
    //5.开始扫描，并保存扫描图像，扫描结束取消扫描
    qDebug()<<"Scanning.........";
    int count = 0;
    SANE_Status status = SANE_STATUS_GOOD;
    while(status == SANE_STATUS_GOOD)
    {
        status = sane->StartScan(sane_handle);
        if(status == SANE_STATUS_NO_DOCS)
        {
            qDebug()<<"扫描结束，共扫描："<<count<<"张纸";
            break;
        }
        else if(status == SANE_STATUS_GOOD)
        {
            QString path = "./"+QString::number(count)+".tif";
            status = sane->SaveScanImage(sane_handle,path,0,0);
            if(status == SANE_STATUS_GOOD)
            {
                count++;
                qDebug()<<"扫描正在进行，第"<<count<<"张纸已保存完成";
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    sane->cancle_scan(sane_handle);
}

void MainWindow::on_pushButton_6_clicked()
{
    //7.关闭设备
    qDebug()<<"Close the device";
    sane->close_device(sane_handle);
}

void MainWindow::on_pushButton_7_clicked()
{
    //8.释放资源
     qDebug()<<"执行Exit";
     sane->exit();
}

void MainWindow::on_pushButton_8_clicked()
{
    //参数描述
    SANE_Option_Descriptor* paramdsr = sane->get_parameters_dsr(sane_handle, ui->lineEdit->text().toInt());
    qDebug()<<"name:"<<paramdsr->name<<" title:"<<paramdsr->title<<" desc:"<<paramdsr->desc<<" type:"<<paramdsr->type;
}

void MainWindow::on_pushButton_9_clicked()
{
    //扫描属性描述
    sane->get_paramters(sane_handle, &sane_param);
    qDebug()<<"format:"<<sane_param.format<<" last_frame"<<sane_param.last_frame<<" bytes_per_line"<<sane_param.bytes_per_line
                     <<" pixels_per_line"<<sane_param.pixels_per_line<<" lines"<<sane_param.lines<<" depth"<<sane_param.depth;
}

void MainWindow::on_pushButton_11_clicked()
{
    //设置参数自动，没用过不理解
    if(sane_status!=SANE_STATUS_GOOD)
    {
        return;
    }
    //获取参数值
    SANE_Int info;
    char valuestr[128];
    int valueint = 0;
    int param = ui->lineEdit->text().toInt();
    if(!m_optmap_type.contains(param))
    {
        qDebug()<<"请检查参数序号是否正确。";
        return;
    }

    switch (m_optmap_type.value(param)) {
    case 0:
    case 1:
    case 2:
        sane->set_option_auto(sane_handle,param,&valueint,&info);
        qDebug()<<m_optmap_name.value(param)+"参数值int："<<valueint;
        ui->lineEdit_2->setText(QString::number(valueint));
        break;
    case 3:
        sane->set_option_auto(sane_handle,param,valuestr,&info);
        qDebug()<<m_optmap_name.value(param)+"参数值str："<<valuestr;
        ui->lineEdit_2->setText(valuestr);
    default:
        break;
    }
}

