#ifndef LANCHATAPP_H
#define LANCHATAPP_H

#include <QApplication>
#include <QUuid>
#include <QHostAddress>

#include "GJson.h"

class LanChatAppPrivate;
class LanChatApp : public QApplication
{
  Q_OBJECT

public:
  LanChatApp(int &argc, char **argv);
  ~LanChatApp();

public:
  QIcon iconMain();
  QIcon iconUserOnline();
  QIcon iconUserOffline();
  QIcon iconMessageBlinkOn();
  QIcon iconMessageBlinkOff();
  QIcon iconMessageBlinkCurrent();

  bool isInitialized() const;
  const QString& errorString() const;

  int notifyInternal() const;

  QUuid userUuid() const;
  QString exposedName() const;
  void setExposedName(QString exposed_name);

public:
  void sendDatagram(const QHostAddress& host, const QByteArray& data);

signals:
  void nonEncryptedDatagram(QHostAddress host, GJson json);
  void encryptedDatagram(QHostAddress host, QByteArray datagram,
                         int uncompressed_size);
  void userIsOnLine(QUuid uuid, QString name, QHostAddress host);
  void userIsOffLine(QUuid uuid);

  void iconBlinks();

private:
  LanChatAppPrivate *d;

  friend class LanChatAppPrivate;
};

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<LanChatApp*>(QCoreApplication::instance()))

#endif // LANCHATAPP_H
