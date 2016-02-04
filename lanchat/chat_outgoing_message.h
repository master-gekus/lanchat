#ifndef CHAT_OUTGOING_MESSAGE_H
#define CHAT_OUTGOING_MESSAGE_H

#include <QWidget>

namespace Ui {
  class ChatOutgoingMessage;
}

class ChatOutgoingMessage : public QWidget
{
  Q_OBJECT

public:
  explicit ChatOutgoingMessage(QWidget *parent = 0);
  ~ChatOutgoingMessage();

private:
  Ui::ChatOutgoingMessage *ui;
};

#endif // CHAT_OUTGOING_MESSAGE_H
