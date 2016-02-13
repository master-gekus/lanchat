#include <QMap>
#include <QSettings>
#include <QCloseEvent>

#include "GUiHelpers.h"

#include "app.h"
#include "main_window.h"

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
  ui->splitter->setSizes({300,20});
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
ChatWindow::findWindow(const QUuid& uuid)
{
  auto it = chat_windows.constFind(uuid);
  if (chat_windows.constEnd() == it)
    return 0;

  return it.value();
}

ChatWindow*
ChatWindow::showWindow(MainWindow* parent, const QUuid& uuid)
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
