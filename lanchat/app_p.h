#ifndef LANCHATAPP_P_H
#define LANCHATAPP_P_H

#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>

#include "GJson.h"

#include "app.h"
#include "encrypted_message.h"

class LanChatAppPrivate : public QObject
{
  Q_OBJECT

private:
  LanChatAppPrivate(LanChatApp *owner);
  ~LanChatAppPrivate();

private:
  LanChatApp *owner_;

  bool initialized_;
  QString error_string_;

  QUuid user_uuid_;
  QString exposed_name_;

  QUdpSocket* socket_;

  QTimer notify_presence_timer_;

  EncryptedMessageManager emm_;

private:
  void setExposedName(QString exposed_name);

signals:

private slots:
  void notify_presence();
  void socket_ready_read();

private slots:
  void processActionNotifyOnline(const QHostAddress& host, const GJson& json);
  void processActionNotifyOffline(const QHostAddress& host, const GJson& json);

// Helpers
private:
  void broadcastMessage(const GJson& json);
  void process_datagram(const QHostAddress& host, const QByteArray& datagramm);

  friend class LanChatApp;
};

Q_DECLARE_METATYPE(QHostAddress)

#endif // LANCHATAPP_P_H

