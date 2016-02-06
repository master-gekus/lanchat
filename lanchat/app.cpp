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
  QByteArray msg("This is a broadcast message!");
  for (LanChatAppPrivate::SockAndAddress const& sock : d->sockets_)
    {
      quint64 len = sock.socket_->writeDatagram(msg, sock.addr_.broadcast(), LANCHAT_PORT);
      qDebug("Sent %d bytes.", (int) len);
    }
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
  SubnetSet new_set = SubnetSet::allSubnets();

  if (current_subnet_set_ != new_set)
    {
      current_subnet_set_ = new_set;
      sockets_.clear();

      QStringList errors;
      for (QNetworkAddressEntry entry : current_subnet_set_)
        {
          QHostAddress addr = entry.ip();
          QUdpSocket *socket = new QUdpSocket();
          if (!socket->bind(addr, LANCHAT_PORT))
            {
              errors.append(trUtf8("%1:%2: %s").arg(addr.toString())
                            .arg(LANCHAT_PORT).arg(socket->errorString()));
              delete socket;
              continue;
            }
          connect(socket, SIGNAL(readyRead()), this, SLOT(socket_ready_read()),
                  Qt::QueuedConnection);
          sockets_.append(SockAndAddress(socket, entry));
        }

      if (!errors.isEmpty())
        emit owner_->bindErrors(errors);
    }
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

      qDebug("Datagram from %s:%d: \"%s\"",
             host_address.toString().toUtf8().constData(), port,
             datagramm.constData());
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
