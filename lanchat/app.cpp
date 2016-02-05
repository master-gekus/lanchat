#include <QIcon>

#include "app.h"
#include "app_p.h"

// ////////////////////////////////////////////////////////////////////////////
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
}

LanChatAppPrivate::~LanChatAppPrivate()
{
}
