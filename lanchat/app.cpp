#include <QIcon>

#include "app.h"

LanChatApp::LanChatApp(int& argc, char** argv) :
  QApplication(argc, argv)
{
}

LanChatApp::~LanChatApp()
{
}

namespace
{
  QIcon main_icon_;
}

QIcon
LanChatApp::getMainIcon()
{
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
