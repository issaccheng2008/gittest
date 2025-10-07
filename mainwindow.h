#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void updateWindowTitle();

private:
    Ui::MainWindow *ui;
    QString m_currentFilePath;

    bool maybeSave();
    bool loadFile(const QString &filePath);
    bool writeFile(const QString &filePath);
};
#endif // MAINWINDOW_H
