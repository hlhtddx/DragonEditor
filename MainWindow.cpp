#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "SettingsDialog.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
      , settings("hlhtddx.net", "DragonEditor")
{
    gameFolderPath = settings.value("gameFolderPath", "").toString();
    if (!openGameFolderPath(gameFolderPath))
    {
        qDebug() << "Invalid game folder path.";
        gameFolderPath.clear();
    }

    dosboxExePath = settings.value("dosboxExePath", "").toString();
    ui->setupUi(this);
    ui->actionLaunch->setDisabled(gameFolderPath.isEmpty() || dosboxExePath.isEmpty());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionPreference_triggered()
{
    qDebug("settings");
    SettingsDialog settingsDialog(this);
    settingsDialog.exec();
}

void MainWindow::on_actionOpenGameFolder_triggered()
{
    qDebug() << "open game folder";
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (fileDialog.exec() == QDialog::Accepted)
    {
        QString selectedPath = fileDialog.selectedFiles().first();
        if (selectedPath == gameFolderPath)
        {
            qDebug() << "game folder path is same.";
            return;
        }
        qDebug() << "Selected Path:" << selectedPath;
        if (!openGameFolderPath(selectedPath))
        {
            qDebug() << "Invalid game folder selected.";
            return;
        }
        gameFolderPath = selectedPath;
        settings.setValue("gameFolderPath", gameFolderPath);
    }
}

void MainWindow::on_actionLaunch_triggered()
{
    if (gameFolderPath.isEmpty())
    {
        qDebug() << "Game folder path is empty.";
        QMessageBox::warning(this, "Warning", "Please select a game folder first.");
        return;
    }
    qDebug() << "Launching game from folder:" << gameFolderPath;
    // Add code to launch the game using dosboxExePath and gameFolderPath
    // For example:
    // QString command = QString("%1 -c \"mount c: %2\" -c \"c:\" -c \"game.exe\"").arg(dosboxExePath, gameFolderPath);
    // QProcess::startDetached(command);
    QMessageBox::information(this, "Information", "Game launched successfully.");
}

bool MainWindow::openGameFolderPath(const QString& path) const
{
    if (path.isEmpty())
    {
        return false;
    }
    if (path == gameFolderPath)
    {
        return false;
    }
    return true;
}
