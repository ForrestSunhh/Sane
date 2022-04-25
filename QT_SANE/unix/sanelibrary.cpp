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
#include "sanelibrary.h"

//using namespace Log4Qt;

QMutex SaneLibrary::_mutex;

SaneLibrary::SaneLibrary(QObject* parent)
  : ScanLibrary(parent)
  , m_scanning(false)
{
  auto* thread = new QThread;
  auto* scan_worker = new SaneWorker();

  // cleanup
  connect(this, &SaneLibrary::finished, thread, &QThread::quit);
  connect(thread, &QThread::finished, scan_worker, &SaneWorker::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);

  // worker
  connect(this, &SaneLibrary::startScanning, scan_worker, &SaneWorker::scan);
  connect(this,
          &SaneLibrary::getAvailableOptions,
          scan_worker,
          &SaneWorker::loadAvailableScannerOptions);
  connect(
    this, &SaneLibrary::cancelScanning, scan_worker, &SaneWorker::cancelScan);
  connect(
    this, &SaneLibrary::setBoolValue, scan_worker, &SaneWorker::setBoolValue);
  connect(
    this, &SaneLibrary::setIntValue, scan_worker, &SaneWorker::setIntValue);
  connect(this,
          &SaneLibrary::setStringValue,
          scan_worker,
          &SaneWorker::setStringValue);
  connect(scan_worker, &SaneWorker::optionsSet, this, &ScanLibrary::optionsSet);
  connect(
    scan_worker, &SaneWorker::sourceChanged, this, &ScanLibrary::sourceChanged);
  connect(
    scan_worker, &SaneWorker::modeChanged, this, &ScanLibrary::modeChanged);
  connect(
    scan_worker, &SaneWorker::scanCompleted, this, &ScanLibrary::scanCompleted);
  connect(scan_worker,
          &SaneWorker::scanCompleted,
          this,
          &SaneLibrary::scanIsCompleted);
  connect(scan_worker, &SaneWorker::scanFailed, this, &ScanLibrary::scanFailed);
  connect(
    scan_worker, &SaneWorker::scanProgress, this, &ScanLibrary::scanProgress);

  scan_worker->moveToThread(thread);
  thread->start();
}

SaneLibrary::~SaneLibrary()
{
  emit finished();
}

bool SaneLibrary::init()
{
  QMutexLocker locker(&_mutex);
  SANE_Int version_code = 0;
  SANE_Status status;
  status = sane_init(&version_code, callbackWrapper);

  if (status == SANE_STATUS_GOOD) {
    m_version = Version(version_code);
    qInfo() << tr("SANE version code: %1").arg(version_code);
    return true;
  }

  return false;
}

QStringList SaneLibrary::devices()
{
  QMutexLocker locker(&_mutex);
  QStringList list;
  SANE_Status sane_status = SANE_STATUS_GOOD;
  const SANE_Device** device_list = nullptr;
  sane_status = sane_get_devices(&device_list, SANE_FALSE);

  if (device_list) {
    qDebug() <<
             tr("sane_get_devices status: %1").arg(sane_strstatus(sane_status));

    const SANE_Device* current_device;
    size_t current_device_index = 0;

    while ((current_device = device_list[current_device_index]) != nullptr) {
      if (!current_device) {
        qDebug() << tr("No devices connected");

      } else {
        auto* scanner = new ScanDevice(this);
        scanner->name = current_device->name;
        scanner->vendor = current_device->vendor;
        scanner->model = current_device->model;
        scanner->type = current_device->type;
        m_scanners.insert(scanner->name, scanner);
        list.append(scanner->name);
      }

      ++current_device_index;
    }
  }

  return list;
}

ScanDevice* SaneLibrary::device(QString device_name)
{
  QMutexLocker locker(&_mutex);
  return m_scanners.value(device_name);
}

// ScanOptions
// SaneLibrary::options(QString device_name)
//{
//  return device->options->value(device_name);
//}

bool SaneLibrary::detectAvailableOptions(QString device_name)
{
  QMutexLocker locker(&_mutex);
  SANE_Status sane_status;
  SANE_Handle sane_handle;
  sane_status = sane_open(device_name.toStdString().c_str(), &sane_handle);

  if (sane_status == SANE_STATUS_GOOD) {
    //    ScanDevice* device = m_scanners.value(device_name);

    qDebug() <<
             tr("sane_open status: %1").arg(sane_strstatus(sane_status));

    sane_close(sane_handle);

    return true;
  }

  return false;
}

bool SaneLibrary::startScan(QString device_name)
{
//  QMutexLocker locker(&_mutex);
  ScanDevice* device = m_scanners.value(device_name);
  qDebug()<<"startscan";
  m_scanning = true;
  emit startScanning(device);

  return false;
}

void SaneLibrary::cancelScan(/*QString device_name*/)
{
  QMutexLocker locker(&_mutex);
  //  ScanDevice* device = m_scanners.value(device_name);
  emit cancelScanning();
  m_scanning = false;
}

void SaneLibrary::exit()
{
  sane_exit();
}

void SaneLibrary::getAvailableScannerOptions(QString device_name)
{
  auto* device = m_scanners.value(device_name);

  emit getAvailableOptions(device);
}

void SaneLibrary::receiveIntValue(ScanDevice* device, int value)
{
  if (device->op_name == SANE_NAME_SCAN_TL_X) {
    device->options->setTopLeftX(value);

  } else if (device->op_name == SANE_NAME_SCAN_TL_Y) {
    device->options->setTopLeftY(value);

  } else if (device->op_name == SANE_NAME_SCAN_BR_X) {
    device->options->setBottomRightX(value);

  } else if (device->op_name == SANE_NAME_SCAN_BR_Y) {
    device->options->setBottomRightY(value);

  } else if (device->op_name == SANE_NAME_CONTRAST) {
    device->options->setContrast(value);

  } else if (device->op_name == SANE_NAME_BRIGHTNESS) {
    device->options->setBrightness(value);

  } else if (device->op_name == SANE_NAME_SCAN_RESOLUTION) {
    device->options->setResolution(value);

  } else if (device->op_name == SANE_NAME_SCAN_X_RESOLUTION) {
    device->options->setResolutionX(value);

  } else if (device->op_name == SANE_NAME_SCAN_Y_RESOLUTION) {
    device->options->setResolutionY(value);
  }
}

void SaneLibrary::scanIsCompleted()
{
  m_scanning = false;
}

QRect SaneLibrary::geometry(QString device_name)
{
  ScanDevice* device = m_scanners.value(device_name);
  return device->options->geometry();
}

const Version& SaneLibrary::version() const
{
  return m_version;
}

// void
// SaneLibrary::topLeftX(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_TL_X);
//  device->op_name = QString(SANE_NAME_SCAN_TL_X);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setTopLeftX(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_TL_X);
  emit setIntValue(device, option_id, QString(SANE_NAME_SCAN_TL_X), value);
}

// void
// SaneLibrary::topLeftY(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_TL_Y);
//  device->op_name = QString(SANE_NAME_SCAN_TL_Y);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setTopLeftY(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_TL_Y);
  emit setIntValue(device, option_id, QString(SANE_NAME_SCAN_TL_Y), value);
}

// void
// SaneLibrary::bottomRightX(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_BR_X);
//  device->op_name = QString(SANE_NAME_SCAN_BR_X);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setBottomRightX(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_BR_X);
  emit setIntValue(device, option_id, QString(SANE_NAME_SCAN_BR_X), value);
}

// void
// SaneLibrary::bottomRightY(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_BR_Y);
//  device->op_name = QString(SANE_NAME_SCAN_BR_Y);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setBottomRightY(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_BR_Y);
  emit setIntValue(device, option_id, QString(SANE_NAME_SCAN_BR_Y), value);
}

// void
// SaneLibrary::contrast(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_CONTRAST);
//  device->op_name = QString(SANE_NAME_CONTRAST);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setContrast(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_CONTRAST);
  emit setIntValue(device, option_id, QString(SANE_NAME_CONTRAST), value);
}

// void
// SaneLibrary::brightness(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_BRIGHTNESS);
//  device->op_name = QString(SANE_NAME_BRIGHTNESS);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setBrightness(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_BRIGHTNESS);
  emit setIntValue(device, option_id, QString(SANE_NAME_BRIGHTNESS), value);
}

// void
// SaneLibrary::resolution(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_RESOLUTION);
//  device->op_name = QString(SANE_NAME_SCAN_RESOLUTION);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setResolution(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_RESOLUTION);
  emit setIntValue(
    device, option_id, QString(SANE_NAME_SCAN_RESOLUTION), value);
}

// void
// SaneLibrary::resolutionX(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_X_RESOLUTION);
//  device->op_name = QString(SANE_NAME_SCAN_X_RESOLUTION);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setResolutionX(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_X_RESOLUTION);
  emit setIntValue(
    device, option_id, QString(SANE_NAME_SCAN_X_RESOLUTION), value);
}

// void
// SaneLibrary::resolutionY(ScanDevice* device, int& value)
//{
//  int option_id = device->options->optionId(SANE_NAME_SCAN_Y_RESOLUTION);
//  device->op_name = QString(SANE_NAME_SCAN_Y_RESOLUTION);
//  emit getIntValue(device, option_id, value);
//}

void SaneLibrary::setResolutionY(ScanDevice* device, int value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_Y_RESOLUTION);
  emit setIntValue(
    device, option_id, QString(SANE_NAME_SCAN_Y_RESOLUTION), value);
}

void SaneLibrary::setPreview(ScanDevice* device)
{
  int option_id = device->options->optionId(SANE_NAME_PREVIEW);

  emit setBoolValue(device, option_id, QString(SANE_NAME_PREVIEW), SANE_TRUE);
}

void SaneLibrary::clearPreview(ScanDevice* device)
{
  int option_id = device->options->optionId(SANE_NAME_PREVIEW);

  emit setBoolValue(device, option_id, QString(SANE_NAME_PREVIEW), SANE_FALSE);
}

void SaneLibrary::setMode(ScanDevice* device, const QString& value)
{
  emit setStringValue(device, QString(SANE_NAME_SCAN_MODE), value);
}

void SaneLibrary::setSource(ScanDevice* device, const QString& value)
{
  int option_id = device->options->optionId(SANE_NAME_SCAN_SOURCE);

  emit setStringValue(device, QString(SANE_NAME_SCAN_SOURCE), value);
}

bool SaneLibrary::isScanning() const
{
  return m_scanning;
}

void SaneLibrary::callbackWrapper(SANE_String_Const resource,
                                  SANE_Char* name,
                                  SANE_Char* password)
{
  // TODO some form of authorisation ???
  //  std::string name_destination;
  //  std::string password_destination;
  //  _callback(std::string(resource), name_destination,
  //  password_destination);
  //  assert(name_destination.size() < SANE_MAX_USERNAME_LEN);
  //  assert(password_destination.size() < SANE_MAX_PASSWORD_LEN);
  //  strncpy(username, name_destination.c_str(), name_destination.size());
  //  strncpy(password, password_destination.c_str(),
  //  password_destination.size());
}

static void auth_callback(SANE_String_Const resource,
                          SANE_Char* username,
                          SANE_Char* password)
{
  // TODO some form of authorisation ???
  //  std::string name_destination;
  //  std::string password_destination;
  //  _callback(std::string(resource), name_destination,
  //  password_destination);
  //  assert(name_destination.size() < SANE_MAX_USERNAME_LEN);
  //  assert(password_destination.size() < SANE_MAX_PASSWORD_LEN);
  //  strncpy(username, name_destination.c_str(), name_destination.size());
  //  strncpy(password, password_destination.c_str(),
  //  password_destination.size());
}
