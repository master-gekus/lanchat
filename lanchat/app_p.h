#ifndef LANCHATAPP_P_H
#define LANCHATAPP_P_H

#include <QUdpSocket>

#include "app.h"

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

private:
  void setExposedName(QString exposed_name);

signals:

private slots:

  friend class LanChatApp;
};


#endif // LANCHATAPP_P_H

