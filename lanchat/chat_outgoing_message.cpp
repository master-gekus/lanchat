#include "chat_outgoing_message.h"
#include "ui_chat_outgoing_message.h"

ChatOutgoingMessage::ChatOutgoingMessage(const QString& text) :
  QWidget(0),
  ui(new Ui::ChatOutgoingMessage)
{
  ui->setupUi(this);
  ui->labelMessage->setText(text);
}

ChatOutgoingMessage::~ChatOutgoingMessage()
{
  delete ui;
}

QLabel*
ChatOutgoingMessage::labelStatus() const
{
  return ui->labelStatus;
}

QString
ChatOutgoingMessage::messageText() const
{
  return ui->labelMessage->text();
}
