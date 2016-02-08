#include <QMessageBox>

#include "app.h"

#include "settings_dialog.h"
#include "ui_settings_dialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SettingsDialog)
{
  ui->setupUi(this);

  ui->editExposedName->setText(qApp->exposedName());
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

void
SettingsDialog::done(int code)
{
  if (Accepted != code)
    {
      QDialog::done(code);
      return;
    }

  QString exposed_name = ui->editExposedName->text().trimmed();
  if (exposed_name.isEmpty())
    {
      QMessageBox::critical(this, this->windowTitle(),
                            QStringLiteral("Exposed name can not be empty!"));
      ui->tabWidget->setCurrentWidget(ui->tabCommon);
      ui->editExposedName->setFocus();
      return;
    }
  qApp->setExposedName(exposed_name);

  QDialog::done(code);
}
