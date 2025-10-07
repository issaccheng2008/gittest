#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTextDocument>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    connect(ui->plainTextEdit->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);

    connect(ui->actionUndo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);

    connect(ui->plainTextEdit, &QPlainTextEdit::copyAvailable, ui->actionCut, &QAction::setEnabled);
    connect(ui->plainTextEdit, &QPlainTextEdit::copyAvailable, ui->actionCopy, &QAction::setEnabled);
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);

    newFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (!maybeSave()) {
        return;
    }

    ui->plainTextEdit->clear();
    ui->plainTextEdit->document()->setModified(false);
    m_currentFilePath.clear();
    updateWindowTitle();
    statusBar()->showMessage(tr("New document"), 2000);
}

void MainWindow::openFile()
{
    if (!maybeSave()) {
        return;
    }

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                          tr("Text Files (*.txt);;All Files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    if (loadFile(filePath)) {
        statusBar()->showMessage(tr("Opened \"%1\"").arg(QFileInfo(filePath).fileName()), 2000);
    }
}

void MainWindow::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        saveFileAs();
    } else if (writeFile(m_currentFilePath)) {
        statusBar()->showMessage(tr("Saved"), 2000);
    }
}

void MainWindow::saveFileAs()
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), m_currentFilePath,
                                                          tr("Text Files (*.txt);;All Files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    if (writeFile(filePath)) {
        statusBar()->showMessage(tr("Saved \"%1\"").arg(QFileInfo(filePath).fileName()), 2000);
    }
}

void MainWindow::updateWindowTitle()
{
    const QString fileName = m_currentFilePath.isEmpty()
                                 ? tr("Untitled")
                                 : QFileInfo(m_currentFilePath).fileName();
    const QString modifiedMarker = ui->plainTextEdit->document()->isModified() ? QStringLiteral("*")
                                                                               : QString();
    setWindowTitle(tr("%1%2 - Notepad").arg(fileName, modifiedMarker));
}

bool MainWindow::maybeSave()
{
    if (!ui->plainTextEdit->document()->isModified()) {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        this,
        tr("Unsaved Changes"),
        tr("The document has been modified. Do you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        saveFile();
        return !ui->plainTextEdit->document()->isModified();
    }

    if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

bool MainWindow::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not open file \"%1\": %2")
                                 .arg(QFileInfo(filePath).fileName(), file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    ui->plainTextEdit->setPlainText(in.readAll());
    ui->plainTextEdit->document()->setModified(false);
    m_currentFilePath = filePath;
    updateWindowTitle();
    return true;
}

bool MainWindow::writeFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not write to file \"%1\": %2")
                                 .arg(QFileInfo(filePath).fileName(), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << ui->plainTextEdit->toPlainText();
    file.close();

    ui->plainTextEdit->document()->setModified(false);
    m_currentFilePath = filePath;
    updateWindowTitle();
    return true;
}
