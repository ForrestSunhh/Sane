/*
    Copyright © Simon Meaden 2019.
    This file was developed as part of the QScan cpp library but could
    easily be used elsewhere.

    QScan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QScan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QScan.  If not, see <http://www.gnu.org/licenses/>.
    It is also available on request from Simon Meaden simonmeaden@sky.com.
*/
#ifndef SANESCANETHREAD_H
#define SANESCANETHREAD_H

#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QDebug>
//#include "logger.h"

#include "scaninterface.h"
#include "scanoptions.h"

class SaneWorker : public QObject
{
  Q_OBJECT
public:
  explicit SaneWorker(QObject* parent = nullptr);

  void scan(ScanDevice* device);
  void loadAvailableScannerOptions(ScanDevice* device);
  void setBoolValue(ScanDevice* device,
                    int option_id,
                    const QString&,
                    bool value);
  void setIntValue(ScanDevice* device,
                   int option_id,
                   const QString& name,
                   int value);
  void setStringValue(ScanDevice* device,
                      const QString& name,
                      const QString& value);
  void cancelScan();

signals:
    void scanCompleted(const QImage &, const int resolution);
    void scanFailed();
    void scanProgress(double);
    void finished();
    //  void availableScannerOptions(ScanDevice*);
    //  void sendIntValue(ScanDevice*, int);
    void optionsSet(ScanDevice *);
    void sourceChanged(ScanDevice *);
    void modeChanged(ScanDevice *);

//    void log(LogLevel, const QString &);

protected:
  //  Log4Qt::Logger* m_logger;
  QMutex m_mutex;
  SANE_Handle m_sane_handle{};

  void getIntValue(ScanDevice* device, int option_id, const QString& name);
  void getListValue(ScanDevice* device,
                    int option_id,
                    const QString& name,
                    const SANE_Option_Descriptor* opt);
  void setResolution(ScanDevice* device,
                     const SANE_Option_Descriptor* current_option,
                     SANE_Int option_id);

  static const int GUARDS_SIZE = 4; /* 4 bytes */
  void* guardedMalloc(size_t size);
  void guardedFree(void* ptr);
  QVariant getOptionValue(ScanDevice* device, const QString& option_name);
};

#endif // SANESCANETHREAD_H
