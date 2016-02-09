#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QHostAddress>

#include "GJson.h"

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

private slots:
  void on_actionAbout_triggered();
  void on_actionSettings_triggered();

  void nonEncryptedDatagram(QHostAddress host, GJson json);

};

#endif // MAIN_WINDOW_H
