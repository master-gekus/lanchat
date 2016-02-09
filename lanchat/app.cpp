#include <QIcon>
#include <QSettings>
#include <QHostInfo>

#include "app.h"
#include "app_p.h"

#define LANCHAT_PORT 7251

#define USER_DATA_GROUP QStringLiteral("User Data")
#define USER_UUID QStringLiteral("uuid")
#define USER_EXPOSED_NAME QStringLiteral("Exposed Name")

// ////////////////////////////////////////////////////////////////////////////
// Constructor LanChatApp::LanChatApp(int& argc, char** argv) moved to the
// bottom of the file. See remark below.
LanChatApp::~LanChatApp()
{
  delete d;
}

QIcon
LanChatApp::getMainIcon()
{
  static QIcon main_icon_;
  if (main_icon_.isNull())
    {
      main_icon_.addFile(":/res/main_icon/16x16.png");
      main_icon_.addFile(":/res/main_icon/32x32.png");
      main_icon_.addFile(":/res/main_icon/48x48.png");
      main_icon_.addFile(":/res/main_icon/64x64.png");
      main_icon_.addFile(":/res/main_icon/128x128.png");
    }

  return main_icon_;
}

bool
LanChatApp::isInitialized() const
{
  return d->initialized_;
}

const QString&
LanChatApp::errorString() const
{
  return d->error_string_;
}

QUuid
LanChatApp::userUuid() const
{
  return d->user_uuid_;
}

QString
LanChatApp::exposedName() const
{
  return d->exposed_name_;
}

void
LanChatApp::setExposedName(QString exposed_name)\
{
  d->setExposedName(exposed_name);
}

// ////////////////////////////////////////////////////////////////////////////
LanChatAppPrivate::LanChatAppPrivate(LanChatApp *owner) :
  owner_(owner),
  initialized_(false),
  error_string_(QStringLiteral("No error.")),
  socket_(new QUdpSocket())
{
  QSettings settings;
  settings.beginGroup(USER_DATA_GROUP);
  QUuid uuid = settings.value(USER_UUID).toUuid();
  if (uuid.isNull())
    {
      uuid = QUuid::createUuid();
      settings.setValue(USER_UUID, uuid);
    }

  QString name = settings.value(USER_EXPOSED_NAME).toString().trimmed();
  if (name.isEmpty())
    {
      name = QHostInfo::localHostName().trimmed();
      if (name.isEmpty())
        name = QStringLiteral("<unnamed user>");
      settings.setValue(USER_EXPOSED_NAME, name);
    }

  user_uuid_ = uuid;
  exposed_name_ = name;

  if (!socket_->bind(LANCHAT_PORT, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
    {
      error_string_ = QStringLiteral("Error bind socket to port %1: %2.")
        .arg(LANCHAT_PORT).arg(socket_->errorString());
      return;
    }

  initialized_ = true;
}

LanChatAppPrivate::~LanChatAppPrivate()
{
  if (socket_)
    {
      socket_->close();
      delete socket_;
    }
}

void
LanChatAppPrivate::setExposedName(QString exposed_name)
{
  exposed_name = exposed_name.trimmed();
  if (exposed_name_ == exposed_name)
    return;

  exposed_name_ = exposed_name;

  QSettings settings;
  settings.beginGroup(USER_DATA_GROUP);
  settings.setValue(USER_EXPOSED_NAME, exposed_name_);
}

// ////////////////////////////////////////////////////////////////////////////
// Constructor moved to the bottom of a file because it "breaks down"
// Qt Creator IntelliSence :-(

LanChatApp::LanChatApp(int& argc, char** argv) :
  QApplication(argc, argv),
  d(0)
{
  setOrganizationName(QStringLiteral("Master Gekus"));
  setApplicationName(QStringLiteral("Lan Chat"));
#ifdef Q_CC_MSVC
  setApplicationVersion(QStringLiteral("Version "
                                       QT_UNICODE_LITERAL(GIT_DESCRIBE)
                                       L" from "
                                       QT_UNICODE_LITERAL(__DATE__) L" "
                                       QT_UNICODE_LITERAL(__TIME__)));
#else
  setApplicationVersion(QStringLiteral("Version " GIT_DESCRIBE " from "
                                       __DATE__ " " __TIME__));
#endif
  d = new LanChatAppPrivate(this);
}
