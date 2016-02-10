#include "send_message_dialog.h"
#include "ui_send_message_dialog.h"

SendMessageDialog::SendMessageDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SendMessageDialog)
{
  ui->setupUi(this);
  ui->editMessage->setPlainText(QStringLiteral("This is a test message!"));
}

SendMessageDialog::~SendMessageDialog()
{
  delete ui;
}

QByteArray
SendMessageDialog::messageText() const
{
  return ui->editMessage->toPlainText().toUtf8();
}
