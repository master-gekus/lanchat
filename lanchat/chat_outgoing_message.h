#ifndef CHAT_OUTGOING_MESSAGE_H
#define CHAT_OUTGOING_MESSAGE_H

#include <QWidget>

class QLabel;

namespace Ui {
  class ChatOutgoingMessage;
}

class ChatOutgoingMessage : public QWidget
{
  Q_OBJECT

public:
  QLabel* labelStatus() const;
  QString messageText() const;

public:
  explicit ChatOutgoingMessage(const QString& text);
  ~ChatOutgoingMessage();

private:
  Ui::ChatOutgoingMessage *ui;
};

#endif // CHAT_OUTGOING_MESSAGE_H
