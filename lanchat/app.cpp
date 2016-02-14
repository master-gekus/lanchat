#include <QIcon>
#include <QSettings>
#include <QHostInfo>
#include <QMetaMethod>

#include "message_composer.h"

#include "app.h"
#include "app_p.h"

#define LANCHAT_PORT 7251
#define NOTIFY_INTERVAL 5000
#define ICON_BLING_INTERVAL 500

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
LanChatApp::iconMain()
{
  static QIcon icon;
  if (icon.isNull())
    {
      icon.addFile(QStringLiteral(":/res/main_icon/16x16.png"));
      icon.addFile(QStringLiteral(":/res/main_icon/32x32.png"));
      icon.addFile(QStringLiteral(":/res/main_icon/48x48.png"));
      icon.addFile(QStringLiteral(":/res/main_icon/64x64.png"));
      icon.addFile(QStringLiteral(":/res/main_icon/128x128.png"));
    }
  return icon;
}

QIcon
LanChatApp::iconUserOnline()
{
  static QIcon icon;
  if (icon.isNull())
    {
      icon.addFile(QStringLiteral(":/res/user_online/16x16.png"));
      icon.addFile(QStringLiteral(":/res/user_online/24x24.png"));
      icon.addFile(QStringLiteral(":/res/user_online/32x32.png"));
      icon.addFile(QStringLiteral(":/res/user_online/48x48.png"));
      icon.addFile(QStringLiteral(":/res/user_online/64x64.png"));
      icon.addFile(QStringLiteral(":/res/user_online/128x128.png"));
    }
  return icon;
}

QIcon
LanChatApp::iconUserOffline()
{
  static QIcon icon;
  if (icon.isNull())
    {
      icon.addFile(QStringLiteral(":/res/user_offline/16x16.png"));
      icon.addFile(QStringLiteral(":/res/user_offline/24x24.png"));
      icon.addFile(QStringLiteral(":/res/user_offline/32x32.png"));
      icon.addFile(QStringLiteral(":/res/user_offline/48x48.png"));
      icon.addFile(QStringLiteral(":/res/user_offline/64x64.png"));
      icon.addFile(QStringLiteral(":/res/user_offline/128x128.png"));
    }
  return icon;
}

QIcon
LanChatApp::iconMessageBlinkOn()
{
  static QIcon icon;
  if (icon.isNull())
    {
      icon.addFile(QStringLiteral(":/res/new_msg_on/16x16.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_on/24x24.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_on/32x32.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_on/48x48.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_on/64x64.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_on/128x128.png"));
    }
  return icon;
}

QIcon
LanChatApp::iconMessageBlinkOff()
{
  static QIcon icon;
  if (icon.isNull())
    {
      icon.addFile(QStringLiteral(":/res/new_msg_off/16x16.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_off/24x24.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_off/32x32.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_off/48x48.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_off/64x64.png"));
      icon.addFile(QStringLiteral(":/res/new_msg_off/128x128.png"));
    }
  return icon;
}

QIcon
LanChatApp::iconMessageBlinkCurrent()
{
  return (d->blink_state_) ? iconMessageBlinkOn() : iconMessageBlinkOff();
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

int
LanChatApp::notifyInternal() const
{
  return NOTIFY_INTERVAL;
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

void
LanChatApp::sendDatagram(const QHostAddress& host, const QByteArray& data)
{
  d->socket_->writeDatagram(data, host, LANCHAT_PORT);
}

// ////////////////////////////////////////////////////////////////////////////
LanChatAppPrivate::LanChatAppPrivate(LanChatApp *owner) :
  owner_(owner),
  initialized_(false),
  error_string_(QStringLiteral("No error.")),
  socket_(new QUdpSocket()),
  notify_presence_timer_(this),
  icon_blink_timer_(this),
  blink_state_(false),
  emm_(owner)
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
      QByteArray user_name = qgetenv("USER");
      if (user_name.isEmpty())
        user_name = qgetenv("USERNAME");
      if (user_name.isEmpty())
        user_name = QByteArrayLiteral("<unknown user>");

      QString computer_name = QHostInfo::localHostName().trimmed();
      if (computer_name.isEmpty())
        computer_name = QStringLiteral("<unknown host>");

      name = QStringLiteral("%1 on %2").arg(QString::fromUtf8(user_name),
                                            computer_name);
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
  connect(&notify_presence_timer_, SIGNAL(timeout()), SLOT(notify_presence()),
          Qt::QueuedConnection);
  notify_presence_timer_.start(NOTIFY_INTERVAL);
  QMetaObject::invokeMethod(this, "notify_presence", Qt::QueuedConnection);

  connect(&icon_blink_timer_, SIGNAL(timeout()), SLOT(icon_blink_timer()),
          Qt::QueuedConnection);
  icon_blink_timer_.start(ICON_BLING_INTERVAL);

  initialized_ = true;
}

LanChatAppPrivate::~LanChatAppPrivate()
{
  if (socket_)
    {
      GJson json;
      json["Action"] = "NotifyOffline";
      json["UserUuid"] = user_uuid_.toRfc4122();
      broadcastMessage(json);

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
      process_datagram(QHostAddress(host_address.toIPv4Address()), datagramm);
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
LanChatAppPrivate::process_datagram(const QHostAddress& host,
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
      int uncompressed_size;
      QByteArray uncomposed
        = MessageComposer::uncomposeEncrypted(datagramm, uncompressed_size);

      emit
        owner_->encryptedDatagram(host, uncomposed, uncompressed_size);
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

void
LanChatAppPrivate::icon_blink_timer()
{
  blink_state_ = !blink_state_;
  emit
    owner_->iconBlinks();
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
