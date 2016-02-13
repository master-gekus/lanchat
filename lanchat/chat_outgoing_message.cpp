#include "chat_outgoing_message.h"
#include "ui_chat_outgoing_message.h"

ChatOutgoingMessage::ChatOutgoingMessage(const QString& text) :
  QWidget(0),
  ui(new Ui::ChatOutgoingMessage)
{
  ui->setupUi(this);
  ui->labelMessage->setText(text);
  ui->labelStatus->setText(QStringLiteral("Отправка не удалась <A HREF=\"qqqq\">Повторить</A>"));
}

ChatOutgoingMessage::~ChatOutgoingMessage()
{
  delete ui;
}
