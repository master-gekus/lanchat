#include <QMessageBox>

#include "main_window.h"
#include "app.h"

int main(int argc, char *argv[])
{
  LanChatApp a(argc, argv);

  if (!a.isInitialized())
    {
      QMessageBox::critical(0,
        QStringLiteral("Initialization error :: LanChat"),
        QStringLiteral("Application is not initialized due to the following error:\r\n%1")
          .arg(a.errorString()));
      return (-1);
    }

  MainWindow w;
  w.show();

  return a.exec();
}
