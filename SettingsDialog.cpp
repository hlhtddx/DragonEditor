#include "SettingsDialog.h"
#include "ui_settings.h"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
      , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
