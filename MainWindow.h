#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_actionQuit_triggered();
    void on_actionPreference_triggered();
    void on_actionOpenGameFolder_triggered();
    void on_actionLaunch_triggered();

private:
    Ui::MainWindow* ui;
    QSettings settings;
    QString gameFolderPath;
    QString dosboxExePath;

    bool openGameFolderPath(const QString& path) const;

};
#endif // MAINWINDOW_H
