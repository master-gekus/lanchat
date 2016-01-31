#ifndef LANCHATAPP_H
#define LANCHATAPP_H

#include <QApplication>

class LanChatApp : public QApplication
{
  Q_OBJECT

public:
  LanChatApp(int &argc, char **argv);
  ~LanChatApp();

public:
  static QIcon getMainIcon();
};

#endif // LANCHATAPP_H
