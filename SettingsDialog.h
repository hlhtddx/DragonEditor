#pragma once
#include "QDialog"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class Dialog;
}

QT_END_NAMESPACE


class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

private:
    Ui::Dialog* ui;
};
