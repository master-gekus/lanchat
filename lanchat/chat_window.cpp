#include "chat_incoming_message.h"
#include "chat_outgoing_message.h"

#include "chat_window.h"
#include "ui_chat_window.h"

ChatWindow::ChatWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::ChatWindow)
{
  ui->setupUi(this);

  QTreeWidgetItem *item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  ui->listHistory->setItemWidget(item, 0, new ChatIncomingMessage());

  item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  ui->listHistory->setItemWidget(item, 0, new ChatIncomingMessage());

  item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  ui->listHistory->setItemWidget(item, 0, new ChatOutgoingMessage());

  item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  ui->listHistory->setItemWidget(item, 0, new ChatIncomingMessage());
}

ChatWindow::~ChatWindow()
{
  delete ui;
}
