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
          sockets_.append(SockAndAddress(socket, entry));
        }

      if (!errors.isEmpty())
        emit owner_->bindErrors(errors);
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
