#ifndef LANCHATAPP_H
#define LANCHATAPP_H

#include <QApplication>
#include <QUuid>

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
  bool isInitialized() const;
  const QString& errorString() const;

  QUuid userUuid() const;
  QString exposedName() const;
  void setExposedName(QString exposed_name);


private:
  LanChatAppPrivate *d;

  friend class LanChatAppPrivate;
};

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<LanChatApp*>(QCoreApplication::instance()))

#endif // LANCHATAPP_H
