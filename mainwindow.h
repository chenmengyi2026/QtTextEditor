#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
class CodeEditor;
class FindDialog;
class ReplaceDialog;
class QAction;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void about();
    void findNext(const QString &text, bool cs);
    void doFind(const QString &text);
    void doReplaceOne(const QString &find, const QString &replace);
    void doReplaceAll(const QString &find, const QString &replace);
    void updateCursorPos();
    void textModified();

private:
    void createActions();
    void createMenuToolBar();
    bool maybeSave();
    void loadFile(const QString &path);
    void setCurrentFile(const QString &path);
    void updateWindowTitle();

    CodeEditor *m_editor;
    FindDialog *m_findDlg;
    ReplaceDialog *m_replaceDlg;

    QString m_curFile;
    QAction *m_actCut;
    QAction *m_actCopy;
    QLabel *m_posLabel;
};

#endif // MAINWINDOW_H
