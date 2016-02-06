#ifndef LANCHATAPP_P_H
#define LANCHATAPP_P_H

#include <QObject>
#include <QMap>
#include <QUdpSocket>
#include <QHostAddress>
#include <QSharedPointer>

class LanChatApp;
class LanChatAppPrivate : public QObject
{
  Q_OBJECT

private:
  LanChatAppPrivate(LanChatApp *owner);
  ~LanChatAppPrivate();

private:
  LanChatApp *owner_;
  QUdpSocket socket_;

signals:

private slots:
  void refresh_subnet_set();
  void socket_ready_read();

  friend class LanChatApp;
};


#endif // LANCHATAPP_P_H

