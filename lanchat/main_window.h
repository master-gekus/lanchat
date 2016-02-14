#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QHostAddress>
#include <QUuid>
#include <QTimer>
#include <QSystemTrayIcon>

namespace Ui {
  class MainWindow;
}

class UserListItem;
class GJson;
class QSystemTrayIcon;

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
  QSystemTrayIcon *tray_icon_;

protected:
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE Q_DECL_FINAL;

private slots:
  void on_actionExit_triggered();
  void on_actionAbout_triggered();
  void on_actionSettings_triggered();

  void on_listUsers_itemDoubleClicked(QTreeWidgetItem* item, int);

  void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void onIconBlinks();

private slots:
  void onUserIsOnLine(QUuid uuid, QString name, QHostAddress host);
  void onUserIsOffLine(QUuid uuid);
  void onMessageReceived(const QUuid& sender_uuid, const GJson& json);

  void checkInactivity();

// Helpers
private:
  UserListItem* upsert_user_item(const QUuid& uuid, const QString& name, bool is_online);
};

#endif // MAIN_WINDOW_H
