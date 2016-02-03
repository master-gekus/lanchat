#include "chat_window.h"
#include "ui_chat_window.h"

ChatWindow::ChatWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::ChatWindow)
{
  ui->setupUi(this);
}

ChatWindow::~ChatWindow()
{
  delete ui;
}
