#include "chat_outgoing_message.h"
#include "ui_chat_outgoing_message.h"

ChatOutgoingMessage::ChatOutgoingMessage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ChatOutgoingMessage)
{
  ui->setupUi(this);
}

ChatOutgoingMessage::~ChatOutgoingMessage()
{
  delete ui;
}
