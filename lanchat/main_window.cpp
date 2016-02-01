#include "app.h"
#include "about_box.h"
#include "chat_window.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setWindowIcon(LanChatApp::getMainIcon());

  QMetaObject::invokeMethod(this, "on_actionChatWindow_triggered",
                            Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void
MainWindow::on_actionAbout_triggered()
{
  AboutBox(this).exec();
}

void
MainWindow::on_actionChatWindow_triggered()
{
  ChatWindow *wnd = new ChatWindow();
  wnd->show();
}
