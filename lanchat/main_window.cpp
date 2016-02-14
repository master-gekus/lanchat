#include <QCloseEvent>
#include <QSettings>
#include <QMetaMethod>
#include <QMessageBox>

#include "app.h"
#include "about_box.h"
#include "settings_dialog.h"
#include "user_list_item.h"
#include "chat_window.h"
#include "encrypted_message.h"

#include "main_window.h"
#include "ui_main_window.h"

#define MAIN_WINDOW_GROUP QStringLiteral("Main Window")
#define MAIN_WINDOW_GEOMETRY_KEY QStringLiteral("Geometry")

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  online_header_(0),
  offline_header_(0),
  check_inactivity_timer_(this),
  tray_icon_(0)
{
  ui->setupUi(this);

  setWindowIcon(qApp->iconMain());

  QSettings settings;
  settings.beginGroup(MAIN_WINDOW_GROUP);
  restoreGeometry(settings.value(MAIN_WINDOW_GEOMETRY_KEY).toByteArray());

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

  for (const auto& user_pair : UserListItem::loadItems())
    upsert_user_item(user_pair.first, user_pair.second, false);

  connect(qApp, SIGNAL(userIsOnLine(QUuid,QString,QHostAddress)),
          SLOT(onUserIsOnLine(QUuid,QString,QHostAddress)),
          Qt::QueuedConnection);
  connect(qApp, SIGNAL(userIsOffLine(QUuid)), SLOT(onUserIsOffLine(QUuid)),
          Qt::QueuedConnection);
  connect(gEmm, SIGNAL(messageReceived(QUuid,GJson)),
          SLOT(onMessageReceived(QUuid,GJson)), Qt::QueuedConnection);
  connect(&check_inactivity_timer_, SIGNAL(timeout()), SLOT(checkInactivity()),
          Qt::QueuedConnection);

  if (QSystemTrayIcon::isSystemTrayAvailable())
    {
      qRegisterMetaType<QSystemTrayIcon::ActivationReason>("QSystemTrayIcon::ActivationReason");

      tray_icon_ = new QSystemTrayIcon(qApp->iconMain(), this);
      connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
              SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)),
              Qt::QueuedConnection);
      tray_icon_->show();
    }

  connect(qApp, SIGNAL(iconBlinks()), SLOT(onIconBlinks()),
          Qt::QueuedConnection);

  check_inactivity_timer_.start(1000);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void
MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.beginGroup(MAIN_WINDOW_GROUP);
  settings.setValue(MAIN_WINDOW_GEOMETRY_KEY, saveGeometry());

  if (event->spontaneous() && (0 != tray_icon_))
    {
      event->ignore();
      hide();
    }
  else
    {
      if (QMessageBox::Yes != QMessageBox::question(this,
        QStringLiteral("Lan Chat :: Confirmation"),
        "<B>Lan Chat does not save any messaged history!</B><BR>"
        "<BR>Quitting Lan Chat will cause the loss all of messages (sent and "
        "received), as well as the inability to further message receiving.<BR>"
        "<BR>Are you really sure to quit the application?"))
        {
          event->ignore();
          return;
        }
      event->accept();
      ChatWindow::destroyAllWindows();
    }
}

void
MainWindow::on_actionExit_triggered()
{
  close();
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
MainWindow::on_listUsers_itemDoubleClicked(QTreeWidgetItem* item, int)
{
  UserListItem *uli = dynamic_cast<UserListItem*>(item);
  if (0 == uli)
    return;

  ChatWindow::createWindow(this, uli->uuid(), true);
}

void
MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch(reason)
    {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
      if (!isVisible())
        show();
      if (isMaximized())
        showNormal();
      break;

    default:
      break;
    }
}

void
MainWindow::onIconBlinks()
{
  if (0 != tray_icon_)
    tray_icon_->setIcon(qApp->iconMessageBlinkCurrent());
}

void
MainWindow::onUserIsOnLine(QUuid uuid, QString name, QHostAddress host)
{
  UserListItem *item = upsert_user_item(uuid, name, true);
  if (0 != item)
    item->setHostAddreess(host);
}

void
MainWindow::onUserIsOffLine(QUuid uuid)
{
  upsert_user_item(uuid, QString(), false);
}

void
MainWindow::onMessageReceived(const QUuid& sender_uuid, const GJson& json)
{
  ChatWindow *window = ChatWindow::createWindow(this, sender_uuid, false);
  Q_ASSERT(0 != window);

  QByteArray action = json["Action"].toByteArray();
  if (action.isEmpty())
    {
      qDebug("MainWindow::onMessageReceived(): No action in message.");
      return;
    }

  int method_index = window->metaObject()->indexOfSlot(
    QMetaObject::normalizedSignature("process" + action + "(GJson)"));
  if (0 > method_index)
    {
      qDebug("ChatWindow: No slot for action \"%s\"!", action.constData());
      return;
    }

  window->metaObject()->method(method_index).invoke(window,
                                                    Qt::QueuedConnection,
                                                    Q_ARG(GJson, json));
}

void
MainWindow::checkInactivity()
{
  QList<QUuid> uuids;
  int inactivity_limit = qApp->notifyInternal() * 3 + 1000;
  int last_index = ui->listUsers->indexOfTopLevelItem(offline_header_);
  for (int i = 1; i < last_index; i++)
    {
      UserListItem *item
        = dynamic_cast<UserListItem*>(ui->listUsers->topLevelItem(i));

      Q_ASSERT(0 != item);
      if (item->inactivityMilliseconds() > inactivity_limit)
        uuids.append(item->uuid());
    }

  if (uuids.isEmpty())
    return;

  qDebug("Items became inactive!");

  for (const QUuid& uuid : uuids)
    upsert_user_item(uuid, QString(), false);
}

UserListItem *
MainWindow::upsert_user_item(const QUuid& uuid, const QString& name, bool is_online)
{
  bool is_selected = false;
  UserListItem *item = UserListItem::findItem(uuid);
  if (0 != item)
    {
      item->updateActivity();
      if (name.isEmpty())
        {
          is_online = false;
        }
      else
        {
          if ((item->isOnline() == is_online) && (item->name() == name))
            return item;

          item->setName(name);
        }

      is_selected = item->isSelected();

      ui->listUsers->takeTopLevelItem(ui->listUsers->indexOfTopLevelItem(item));
      item->setOnline(is_online);
    }
  else
    {
      if (name.isEmpty())
        return 0;

      item = new UserListItem(uuid, name, is_online);
    }

  int first_index = is_online ? 1
                    : ui->listUsers->indexOfTopLevelItem(offline_header_) + 1;
  int last_index = is_online
                   ? ui->listUsers->indexOfTopLevelItem(offline_header_)
                   : ui->listUsers->topLevelItemCount();

  // It is highly unlikely that we will have a lot of items. It is not
  // necessary to use any algorithms like a binary insertions
  for(; first_index < last_index; ++first_index)
    {
      UserListItem *candidate
        = dynamic_cast<UserListItem*>(ui->listUsers->topLevelItem(first_index));

      Q_ASSERT(0 != candidate);

      if (candidate->name() > item->name())
        break;
    }

  ui->listUsers->insertTopLevelItem(first_index, item);

  online_header_->setHidden(
    1 == ui->listUsers->indexOfTopLevelItem(offline_header_)
  );

  offline_header_->setHidden(
    (ui->listUsers->topLevelItemCount() - 1)
     == ui->listUsers->indexOfTopLevelItem(offline_header_)
  );

  if (is_selected)
    {
      item->setSelected(true);
      ui->listUsers->setCurrentItem(item);
      ui->listUsers->scrollToItem(item);
    }

  return item;
}
