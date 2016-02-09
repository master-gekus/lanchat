#include <QIcon>
#include <QSettings>
#include <QHostInfo>
#include <QMetaMethod>

#include "message_composer.h"

#include "app.h"
#include "app_p.h"

#define LANCHAT_PORT 7251
#define NOTIFY_INTERVAL 10000

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
  socket_(new QUdpSocket()),
  notify_presence_timer_(this)
{
  qRegisterMetaType<QHostAddress>("QHostAddress");

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

  connect(socket_, SIGNAL(readyRead()), this, SLOT(socket_ready_read()),
          Qt::QueuedConnection);

  Q_ASSERT(!notify_presence_timer_.isSingleShot());
  connect(&notify_presence_timer_, SIGNAL(timeout()), this,
          SLOT(notify_presence()), Qt::QueuedConnection);
  notify_presence_timer_.start(NOTIFY_INTERVAL);
  QMetaObject::invokeMethod(this, "notify_presence", Qt::QueuedConnection);

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

  QMetaObject::invokeMethod(this, "notify_presence", Qt::QueuedConnection);
}

void
LanChatAppPrivate::notify_presence()
{
  GJson json;
  json["Action"] = "NotifyOnline";
  json["UserUuid"] = user_uuid_.toRfc4122();
  json["UserName"] = exposed_name_;
  broadcastMessage(json);
}

void
LanChatAppPrivate::socket_ready_read()
{
  QUdpSocket *socket = dynamic_cast<QUdpSocket*>(sender());
  if ((0 == socket) || (socket != socket_))
    {
      qDebug("LanChatAppPrivate::socket_ready_read(): invalid sender!");
      return;
    }

  while (socket_->hasPendingDatagrams())
    {
      QHostAddress host_address;
      QByteArray datagramm;
      datagramm.resize(socket_->pendingDatagramSize());
      socket_->readDatagram(datagramm.data(), datagramm.size(), &host_address);
      process_datagramm(QHostAddress(host_address.toIPv4Address()), datagramm);
    }
}

void
LanChatAppPrivate::processActionNotifyOnline(const QHostAddress& host,
                                             const GJson& json)
{
  QByteArray raw_uuid = json["UserUuid"].toByteArray();
  if (raw_uuid.size() != 16)
      return;

  QUuid uuid = QUuid::fromRfc4122(raw_uuid);
  if (qApp->userUuid() == uuid)
    return;   // Our own uuid - ignoring!

  QString name = json["UserName"].toString();
  if (name.isEmpty())
    return;

  emit
    owner_->userIsOnLine(uuid, name, host);
}

void
LanChatAppPrivate::processActionNotifyOffline(const QHostAddress& host,
                                              const GJson& json)
{
  Q_UNUSED(host);

  QByteArray raw_uuid = json["UserUuid"].toByteArray();
  if (raw_uuid.size() != 16)
    {
      qDebug("LanChatAppPrivate::processActionNotifyOffline(): invalid UUID.");
      return;
    }

  QUuid uuid = QUuid::fromRfc4122(raw_uuid);
  if (qApp->userUuid() == uuid)
    return;   // Our own uuid - ignoring!

  emit
    owner_->userIsOffLine(uuid);
}

void
LanChatAppPrivate::broadcastMessage(const GJson& json)
{
  socket_->writeDatagram(MessageComposer::composeNonEncrypted(json),
                         QHostAddress::Broadcast, LANCHAT_PORT);
}

void
LanChatAppPrivate::process_datagramm(const QHostAddress& host,
                                     const QByteArray& datagramm)
{
  if (!MessageComposer::isValid(datagramm))
    {
      qDebug("LanChatAppPrivate::process_datagram(): invalid datagram from %s.",
             host.toString().toUtf8().constData());
      return;
    }

  if (MessageComposer::isEncrypted(datagramm))
    {
      qDebug("Encryption is not supported now!");
    }
  else
    {
      GJson const msg = MessageComposer::uncomposeNonEncrypted(datagramm);
      emit
        owner_->nonEncryptedDatagram(host, msg);

      QByteArray action = msg["Action"].toByteArray();
      if (action.isEmpty())
        {
          qDebug("LanChatAppPrivate::process_datagram(): No action in message!");
          return;
        }

      int method_index = metaObject()->indexOfSlot(
        QMetaObject::normalizedSignature("processAction" + action
                                         + "(QHostAddress,GJson)"));
      if (0 > method_index)
        {
          qDebug("LanChatAppPrivate::process_datagram(): "
                 "No slot for action \"%s\"!", action.constData());
          return;
        }

      metaObject()->method(method_index).invoke(this, Qt::QueuedConnection,
                                                Q_ARG(QHostAddress, host),
                                                Q_ARG(GJson, msg));
    }
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
