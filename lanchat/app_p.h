#ifndef LANCHATAPP_P_H
#define LANCHATAPP_P_H

#include "app.h"

class LanChatAppPrivate : public QObject
{
  Q_OBJECT

private:
  LanChatAppPrivate(LanChatApp *owner);
  ~LanChatAppPrivate();

private:
  LanChatApp *owner_;
  QUuid user_uuid_;
  QString exposed_name_;

private:
  void setExposedName(QString exposed_name);

signals:

private slots:

  friend class LanChatApp;
};


#endif // LANCHATAPP_P_H

