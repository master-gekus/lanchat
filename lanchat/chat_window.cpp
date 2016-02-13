#include <QMap>
#include <QSettings>
#include <QCloseEvent>

#include "GUiHelpers.h"

#include "app.h"
#include "encrypted_message.h"
#include "main_window.h"
#include "chat_outgoing_message.h"
#include "chat_incoming_message.h"

#include "chat_window.h"
#include "ui_chat_window.h"

#define CHAT_WINDOW_GROUP QStringLiteral("Chat Windows")
#define CHAT_WINDOW_GEOMETRY_KEY QStringLiteral("Geometry")
#define CHAT_WINDOW_STATE_KEY QStringLiteral("State")

namespace
{
  QMap<QUuid,ChatWindow*> chat_windows;
}

ChatWindow::ChatWindow(const QUuid& uuid) :
  QMainWindow(0),
  ui(new Ui::ChatWindow),
  user_uuid_(uuid)
{
  Q_ASSERT(!chat_windows.contains(user_uuid_));
  chat_windows.insert(user_uuid_, this);

  ui->setupUi(this);
  ui->splitter->setSizes(QList<int>() << 300 << 20);
  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);

  setWindowIcon(qApp->getMainIcon());
}

ChatWindow::~ChatWindow()
{
  delete ui;

  Q_ASSERT(chat_windows.contains(user_uuid_));
  chat_windows.remove(user_uuid_);
}

ChatWindow*
ChatWindow::createWindow(MainWindow* parent, const QUuid& uuid, bool show_window)
{
  ChatWindow *window = 0;

  auto it = chat_windows.constFind(uuid);
  if (chat_windows.constEnd() == it)
    {
      window = new ChatWindow(uuid);

      // TODO: Connect signals/slots
      Q_UNUSED(parent);
    }
  else
    {
      window = it.value();
    }

  if (show_window)
    {
      if (!window->isVisible())
        {
          window->show();

          QSettings settings;
          settings.beginGroup(CHAT_WINDOW_GROUP);
          settings.beginGroup(window->user_uuid_.toString());

          window->restoreGeometry(settings.value(CHAT_WINDOW_GEOMETRY_KEY).toByteArray());
          GUiHelpers::restoreElementsState(window, settings);
        }

      if (window->isMinimized())
        window->showNormal();

      window->activateWindow();
      window->raise();
    }

  return window;
}

void
ChatWindow::destroyAllWindows()
{
  for (ChatWindow *window : chat_windows.values())
    {
      window->close();
      delete window;
    }
}

void
ChatWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.beginGroup(CHAT_WINDOW_GROUP);
  settings.beginGroup(user_uuid_.toString());

  settings.setValue(CHAT_WINDOW_GEOMETRY_KEY, saveGeometry());
  GUiHelpers::saveElementsState(this, settings);

  QMainWindow::closeEvent(event);
}

void
ChatWindow::on_btnSend_clicked()
{
  QString msg = ui->editMessage->toPlainText().trimmed();
  if (msg.isEmpty())
    return;
  ui->editMessage->clear();
  ui->editMessage->setFocus();

  QUuid message_id = QUuid::createUuid();
  GJson json;
  json["Action"] = "NewMessage";
  json["MessageId"] = message_id.toRfc4122();
  json["Message"] = msg;

  gEmm->sendMessage(user_uuid_, json);

  QTreeWidgetItem *item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  ui->listHistory->setItemWidget(item, 0, new ChatOutgoingMessage(msg));
  ui->listHistory->scrollToItem(item);
}

void
ChatWindow::processJson(const GJson& json)
{
  QTreeWidgetItem *item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  ui->listHistory->setItemWidget(item, 0, new ChatIncomingMessage(json["Message"].toString()));
  ui->listHistory->scrollToItem(item);
}
