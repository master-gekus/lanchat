#ifndef LANCHATAPP_H
#define LANCHATAPP_H

#include <QApplication>

#define LANCHAT_PORT 7251

class LanChatAppPrivate;
class LanChatApp : public QApplication
{
  Q_OBJECT

public:
  LanChatApp(int &argc, char **argv);
  ~LanChatApp();

public:
  static QIcon getMainIcon();

public:
  void send_broadcast();

private:
  LanChatAppPrivate *d;

signals:
  void bindErrors(QStringList errors);

  friend class LanChatAppPrivate;
};

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<LanChatApp*>(QCoreApplication::instance()))

#endif // LANCHATAPP_H
