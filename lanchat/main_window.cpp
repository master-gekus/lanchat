#include "app.h"
#include "about_box.h"
#include "settings_dialog.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setWindowIcon(LanChatApp::getMainIcon());

  connect(qApp, SIGNAL(nonEncryptedDatagram(QHostAddress,GJson)), this,
          SLOT(nonEncryptedDatagram(QHostAddress,GJson)), Qt::QueuedConnection);
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
MainWindow::on_actionSettings_triggered()
{
  SettingsDialog(this).exec();
}

void
MainWindow::nonEncryptedDatagram(QHostAddress host, GJson json)
{
  QTreeWidgetItem *item = new QTreeWidgetItem();
  item->setText(0, QStringLiteral("%1: %2").arg(host.toString(),
    QString::fromUtf8(json.toJson(GJson::MinSize))));
  ui->treeWidget->addTopLevelItem(item);
}
