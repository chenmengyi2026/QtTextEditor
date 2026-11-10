#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QColor>

class CodeEditor;
class FindDialog;
class ReplaceDialog;
class QAction;
class QLabel;
class QToolBar;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void loadFile(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void about();

    void findNext(const QString &text, bool caseSensitive);
    void doFind(const QString &text);
    void doReplaceOne(const QString &find, const QString &replace);
    void doReplaceAll(const QString &find, const QString &replace);

    void updateCursorPos();
    void updateStatusInfo();
    void updateWindowTitle();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    bool maybeSave();
    void setCurrentFile(const QString &path);
    void applyColors();

    CodeEditor *m_editor;
    FindDialog *m_findDlg;
    ReplaceDialog *m_replaceDlg;
    QString m_curFile;
    QColor m_textColor;
    QColor m_bgColor;

    QLabel *m_posLabel;
    QLabel *m_infoLabel;
    QToolBar *m_toolBar;

    QAction *m_actNew;
    QAction *m_actOpen;
    QAction *m_actSave;
    QAction *m_actSaveAs;
    QAction *m_actQuit;
    QAction *m_actUndo;
    QAction *m_actRedo;
    QAction *m_actCut;
    QAction *m_actCopy;
    QAction *m_actPaste;
    QAction *m_actSelectAll;
    QAction *m_actFont;
    QAction *m_actTextColor;
    QAction *m_actBgColor;
    QAction *m_actAutoWrap;
    QAction *m_actLineNo;
    QAction *m_actFind;
    QAction *m_actReplace;
    QAction *m_actToolBar;
    QAction *m_actStatusBar;
    QAction *m_actAbout;
};

#endif // MAINWINDOW_H
