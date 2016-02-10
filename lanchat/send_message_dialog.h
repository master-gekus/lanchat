#ifndef SEND_MESSAGE_DIALOG_H
#define SEND_MESSAGE_DIALOG_H

#include <QDialog>

namespace Ui {
  class SendMessageDialog;
}

class SendMessageDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SendMessageDialog(QWidget *parent = 0);
  ~SendMessageDialog();

private:
  Ui::SendMessageDialog *ui;

public:
  QByteArray messageText() const;
};

#endif // SEND_MESSAGE_DIALOG_H
