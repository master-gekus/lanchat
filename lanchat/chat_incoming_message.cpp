#include <QDateTime>

#include "chat_incoming_message.h"
#include "ui_chat_incoming_message.h"

ChatIncomingMessage::ChatIncomingMessage(const QString& text) :
  QWidget(0),
  ui(new Ui::ChatIncomingMessage)
{
  ui->setupUi(this);

  ui->labelMessage->setText(text);
  ui->labelStatus->setText(QDateTime::currentDateTime().toString(
      QStringLiteral("'Received at 'dd.MM.yy HH:mm")));
}

ChatIncomingMessage::~ChatIncomingMessage()
{
  delete ui;
}
