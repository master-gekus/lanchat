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

signals:

private slots:

  friend class LanChatApp;
};


#endif // LANCHATAPP_P_H

