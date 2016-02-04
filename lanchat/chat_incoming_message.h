#ifndef CHAT_INCOMING_MESSAGE_H
#define CHAT_INCOMING_MESSAGE_H

#include <QWidget>

namespace Ui {
  class ChatIncomingMessage;
}

class ChatIncomingMessage : public QWidget
{
  Q_OBJECT

public:
  explicit ChatIncomingMessage(QWidget *parent = 0);
  ~ChatIncomingMessage();

private:
  Ui::ChatIncomingMessage *ui;
};

#endif // CHAT_INCOMING_MESSAGE_H
