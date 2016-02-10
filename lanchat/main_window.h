#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QHostAddress>
#include <QUuid>
#include <QTimer>

#include "encrypted_sessions_manager.h"

namespace Ui {
  class MainWindow;
}

class UserListItem;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  QTreeWidgetItem *online_header_,
                  *offline_header_;
  QTimer check_inactivity_timer_;

private slots:
  void on_actionAbout_triggered();
  void on_actionSettings_triggered();
  void on_actionSendMessage_triggered();

private slots:
  void onUserIsOnLine(QUuid uuid, QString name, QHostAddress host);
  void onUserIsOffLine(QUuid uuid);

  void onSendingResult(EncryptedSessionsManager::MessageID message_id,
                       bool is_ok, QString error_string);

  void checkInactivity();

// Helpers
private:
  UserListItem* upsert_user_item(const QUuid& uuid, const QString& name, bool is_online);
};

#endif // MAIN_WINDOW_H
