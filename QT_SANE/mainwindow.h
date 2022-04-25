#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QLibSane.h"
#include <QPrinterInfo>
#include <QPrinter>
#include <QMap>
#include <QLabel>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

private:
    Ui::MainWindow *ui;
    QLibSane *sane;
    QPrinterInfo* info;
    QStringList devs;


    SANE_Device ** device_list;
    SANE_Status sane_status;
    SANE_Handle sane_handle;
    SANE_Device *device;
    SANE_Parameters  sane_param;
    QMap<int, SANE_Value_Type> m_optmap_type;
    QMap<int, QString> m_optmap_name;

};

#endif // MAINWINDOW_H
