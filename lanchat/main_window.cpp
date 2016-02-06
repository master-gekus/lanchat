#include "app.h"
#include "about_box.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setWindowIcon(LanChatApp::getMainIcon());
  connect(qApp, SIGNAL(datagram_received(QString)), this,
          SLOT(onDatagramReceived(QString)), Qt::QueuedConnection);
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
MainWindow::on_actionSendBroadcast_triggered()
{
  qApp->send_broadcast();
}

void
MainWindow::onDatagramReceived(QString msg)
{
  ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << msg));
}
