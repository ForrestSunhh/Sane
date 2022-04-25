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
#ifndef SCANSANE_H
#define SCANSANE_H

#include <QObject>

#include <QImage>
#include <QMutexLocker>
#include <QThread>
#include <QtDebug>

#include "sane/sane.h"
#include "sane/saneopts.h"

//#include "logger.h"

#include "saneworker.h"
#include "scaninterface.h"
#include "scanoptions.h"
#include "version.h"

#ifndef PATH_MAX
  #define PATH_MAX 1024
#endif

class SaneLibrary final : public ScanLibrary
{
  Q_OBJECT
public:
  explicit SaneLibrary(QObject* parent = nullptr);
  ~SaneLibrary() override;

  bool init() override;
  void exit() override;

  // ScanInterface interface
  QStringList devices() override;
  ScanDevice* device(QString device_name) override;
  //  ScanOptions options(QString device_name) override;
  bool detectAvailableOptions(QString device_name) override;
  bool startScan(QString device_name) override;
  bool isScanning() const override;
  void cancelScan() override;
  QRect geometry(QString device_name) override;
  const Version& version() const;

  //  void topLeftX(ScanDevice* device, int& value) override;
  void setTopLeftX(ScanDevice* device, int x) override;
  //  void topLeftY(ScanDevice* device, int& value) override;
  void setTopLeftY(ScanDevice* device, int x) override;
  //  void bottomRightX(ScanDevice* device, int& value) override;
  void setBottomRightX(ScanDevice* device, int value) override;
  //  void bottomRightY(ScanDevice* device, int& value) override;
  void setBottomRightY(ScanDevice* device, int x) override;
  //  void contrast(ScanDevice* device, int& value) override;
  void setContrast(ScanDevice* device, int value) override;
  //  void brightness(ScanDevice* device, int& value) override;
  void setBrightness(ScanDevice* device, int value) override;
  //  void resolution(ScanDevice* device, int& value) override;
  void setResolution(ScanDevice* device, int value) override;
  //  void resolutionX(ScanDevice* device, int& value) override;
  void setResolutionX(ScanDevice* device, int value) override;
  //  void resolutionY(ScanDevice* device, int& value) override;
  void setResolutionY(ScanDevice* device, int value) override;
  void setPreview(ScanDevice* device) override;
  void clearPreview(ScanDevice* device) override;
  void setMode(ScanDevice* device, const QString& value) override;
  void setSource(ScanDevice* device, const QString& value) override;

signals:
  void finished();
  void startScanning(ScanDevice*);
  void getAvailableOptions(ScanDevice*);
  void setBoolValue(ScanDevice*, int, const QString&, bool);
  void setIntValue(ScanDevice*, int, const QString&, int);
  void setStringValue(ScanDevice*, const QString&, const QString&);
  void getIntValue(ScanDevice*, int, int);
  void cancelScanning();

protected:
  DeviceMap m_scanners;
  QImage* m_image{};
  Version m_version;
  bool m_scanning;

  static QMutex _mutex;

  void getAvailableScannerOptions(QString device_name) override;
  void receiveIntValue(ScanDevice* device, int value);
  void scanIsCompleted();

  static void callbackWrapper(SANE_String_Const resource,
                              SANE_Char* name,
                              SANE_Char* password);

public:
};

#endif // SCANSANE_H
