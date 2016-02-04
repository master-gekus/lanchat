#include "chat_incoming_message.h"
#include "ui_chat_incoming_message.h"

ChatIncomingMessage::ChatIncomingMessage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ChatIncomingMessage)
{
  ui->setupUi(this);
}

ChatIncomingMessage::~ChatIncomingMessage()
{
  delete ui;
}
