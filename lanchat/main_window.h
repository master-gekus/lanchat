#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QHostAddress>
#include <QUuid>

namespace Ui {
  class MainWindow;
}

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

private slots:
  void on_actionAbout_triggered();
  void on_actionSettings_triggered();

private slots:
  void onUserIsOnLine(QUuid uuid, QString name, QHostAddress host);
  void onUserIsOffLine(QUuid uuid);

// Helpers
private:
  void upsert_user_item(const QUuid& uuid, const QString& name, bool is_online);
};

#endif // MAIN_WINDOW_H
