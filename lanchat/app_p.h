#ifndef LANCHATAPP_P_H
#define LANCHATAPP_P_H

#include <QObject>
#include <QUdpSocket>

#include "subnet_set.h"

class LanChatAppPrivate : public QObject
{
  Q_OBJECT

private:
  LanChatAppPrivate();
  ~LanChatAppPrivate();

signals:

private slots:

  friend class LanChatApp;
};


#endif // LANCHATAPP_P_H

