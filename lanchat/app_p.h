#ifndef LANCHATAPP_P_H
#define LANCHATAPP_P_H

#include <QObject>
#include <QMap>
#include <QUdpSocket>
#include <QHostAddress>
#include <QSharedPointer>

#include "subnet_set.h"

class LanChatApp;
class LanChatAppPrivate : public QObject
{
  Q_OBJECT

private:
  LanChatAppPrivate(LanChatApp *owner);
  ~LanChatAppPrivate();

private:
  LanChatApp *owner_;
  SubnetSet current_subnet_set_;

  struct SockAndAddress
  {
    SockAndAddress(QUdpSocket *socket, const QNetworkAddressEntry& addr) :
      socket_(socket),
      addr_(addr)
    {}
    QSharedPointer<QUdpSocket> socket_;
    QNetworkAddressEntry addr_;
  };

  QList<SockAndAddress> sockets_;

signals:

private slots:
  void refresh_subnet_set();
  void socket_ready_read();

  friend class LanChatApp;
};


#endif // LANCHATAPP_P_H

