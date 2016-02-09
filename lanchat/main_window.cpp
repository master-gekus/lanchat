#include "app.h"
#include "about_box.h"
#include "settings_dialog.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  online_header_(0),
  offline_header_(0)
{
  ui->setupUi(this);

  setWindowIcon(LanChatApp::getMainIcon());

  // Creating pre-defined ites
  online_header_ = new QTreeWidgetItem();
  QFont font(online_header_->font(0));
  font.setBold(true);
  QBrush brush = online_header_->foreground(0);

  online_header_->setBackgroundColor(0, QColor(224, 224, 224));
  online_header_->setTextAlignment(0, Qt::AlignCenter);
  online_header_->setData(0, Qt::SizeHintRole, QSize(100, 21));
  online_header_->setFont(0, font);
  online_header_->setText(0, QStringLiteral("On-line"));
  online_header_->setFlags(online_header_->flags() & (~Qt::ItemIsEnabled));
  online_header_->setForeground(0, brush);

  offline_header_ = new QTreeWidgetItem();
  offline_header_->setBackgroundColor(0, QColor(224, 224, 224));
  offline_header_->setTextAlignment(0, Qt::AlignCenter);
  offline_header_->setData(0, Qt::SizeHintRole, QSize(100, 21));
  offline_header_->setFont(0, font);
  offline_header_->setText(0, QStringLiteral("Off-line"));
  offline_header_->setFlags(online_header_->flags() & (~Qt::ItemIsEnabled));
  offline_header_->setForeground(0, brush);

  ui->listUsers->addTopLevelItem(online_header_);
  ui->listUsers->addTopLevelItem(offline_header_);

  online_header_->setHidden(true);
  offline_header_->setHidden(true);
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
