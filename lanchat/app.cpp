#include <QIcon>

#include "app.h"
#include "app_p.h"

// ////////////////////////////////////////////////////////////////////////////
// Look for construcor
// LanChatApp::LanChatApp(int& argc, char** argv)
// at the bottom of file
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

void
LanChatApp::send_broadcast()
{
  static int number = 0;
  QByteArray datagram = "This is a datagram number "
                        + QByteArray::number(++number) + "!";
  quint64 len = d->socket_.writeDatagram(datagram, QHostAddress::Broadcast, LANCHAT_PORT);
  qDebug("Send %d bytes of datagram number %d.", (int) len, number);
}

// ////////////////////////////////////////////////////////////////////////////
LanChatAppPrivate::LanChatAppPrivate(LanChatApp *owner) :
  owner_(owner)
{
  QMetaObject::invokeMethod(this, "refresh_subnet_set", Qt::QueuedConnection);
}

LanChatAppPrivate::~LanChatAppPrivate()
{
}

void
LanChatAppPrivate::refresh_subnet_set()
{
  if (!socket_.bind(LANCHAT_PORT, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress))
    {
      qDebug("Error binding socket: %s", socket_.errorString().toUtf8().constData());
    }
  connect(&socket_, SIGNAL(readyRead()), this, SLOT(socket_ready_read()),
          Qt::QueuedConnection);
}

void
LanChatAppPrivate::socket_ready_read()
{
  QUdpSocket *socket = dynamic_cast<QUdpSocket*>(sender());
  if (0 == socket)
    {
      qDebug("LanChatAppPrivate::socket_ready_read(): invalid sender!");
      return;
    }

  while (socket->hasPendingDatagrams())
    {
      QHostAddress host_address;
      quint16 port;
      QByteArray datagramm;
      datagramm.resize(socket->pendingDatagramSize());
      socket->readDatagram(datagramm.data(), datagramm.size(), &host_address, &port);
      QHostAddress ipV4addr = QHostAddress(host_address.toIPv4Address());

      emit
        owner_->datagram_received(trUtf8("Datagram from %1:%2: \"%3\"")
                                  .arg(ipV4addr.toString()).arg(port)
                                  .arg(QString::fromUtf8(datagramm)));
    }
}

// ////////////////////////////////////////////////////////////////////////////
// I put constructor at the bottom of file because after #ifdef Q_CC_MSVC
// in it Qt Creator IntelliSence does not works properly.
// Possible Qt Creator bug?
LanChatApp::LanChatApp(int& argc, char** argv) :
  QApplication(argc, argv),
  d(new LanChatAppPrivate(this))
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
}
