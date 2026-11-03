#include "mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDialog>
#include <QColorDialog>
#include <QLabel>
#include <QCloseEvent>
#include <QTextCursor>
#include <QFile>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_editor = new CodeEditor(this);
    setCentralWidget(m_editor);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    m_findDlg = new FindDialog(this);
    m_findDlg->hide();
    m_replaceDlg = new ReplaceDialog(this);
    m_replaceDlg->hide();

    createActions();
    createMenuToolBar();

    connect(m_editor, &CodeEditor::modificationChanged, this, &MainWindow::textModified);
    connect(m_editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateCursorPos);

    m_posLabel = new QLabel("行:1 列:1", this);
    statusBar()->addPermanentWidget(m_posLabel);
    statusBar()->showMessage("就绪");

    setCurrentFile("");
    resize(820, 560);
}

void MainWindow::createActions()
{
    // 文件
    connect(new QAction(QIcon::fromTheme("document-new"), "新建", this), &QAction::triggered, this, &MainWindow::newFile);
    connect(new QAction(QIcon::fromTheme("document-open"), "打开", this), &QAction::triggered, this, &MainWindow::openFile);
    QAction *save = new QAction(QIcon::fromTheme("document-save"), "保存", this);
    connect(save, &QAction::triggered, this, &MainWindow::saveFile);
    QAction *saveAs = new QAction("另存为", this);
    connect(saveAs, &QAction::triggered, this, &MainWindow::saveAsFile);
    QAction *quit = new QAction("退出", this);
    connect(quit, &QAction::triggered, this, &QWidget::close);

    // 编辑
    QAction *undo = new QAction("撤销", this);  connect(undo, &QAction::triggered, m_editor, &CodeEditor::undo);
    QAction *redo = new QAction("重做", this);  connect(redo, &QAction::triggered, m_editor, &CodeEditor::redo);
    m_actCut = new QAction("剪切", this);       connect(m_actCut, &QAction::triggered, m_editor, &CodeEditor::cut);
    m_actCopy = new QAction("复制", this);      connect(m_actCopy, &QAction::triggered, m_editor, &CodeEditor::copy);
    QAction *paste = new QAction("粘贴", this); connect(paste, &QAction::triggered, m_editor, &CodeEditor::paste);
    QAction *selectAll = new QAction("全选", this); connect(selectAll, &QAction::triggered, m_editor, &CodeEditor::selectAll);
    undo->setEnabled(false); redo->setEnabled(false);
    connect(m_editor, &CodeEditor::undoAvailable, undo, &QAction::setEnabled);
    connect(m_editor, &CodeEditor::redoAvailable, redo, &QAction::setEnabled);

    // 格式
    QAction *fontAct = new QAction("字体", this);
    connect(fontAct, &QAction::triggered, this, [=]{
        bool ok; QFont f = QFontDialog::getFont(&ok, m_editor->font(), this);
        if (ok) m_editor->setFont(f);
    });
    QAction *colorAct = new QAction("字体颜色", this);
    connect(colorAct, &QAction::triggered, this, [=]{
        QColor c = QColorDialog::getColor(Qt::black, this);
        if (c.isValid()) m_editor->setStyleSheet(QString("QPlainTextEdit{color:%1;}").arg(c.name()));
    });
    QAction *bgAct = new QAction("背景颜色", this);
    connect(bgAct, &QAction::triggered, this, [=]{
        QColor c = QColorDialog::getColor(Qt::white, this);
        if (c.isValid()) m_editor->setStyleSheet(
            QString("QPlainTextEdit{background-color:%1;}").arg(c.name()));
    });
    QAction *wrapAct = new QAction("自动换行", this);
    wrapAct->setCheckable(true); wrapAct->setChecked(true);
    connect(wrapAct, &QAction::toggled, this, [=](bool on){
        m_editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    });

    // 查找替换
    QAction *findAct = new QAction("查找", this);
    connect(findAct, &QAction::triggered, this, [=]{ m_findDlg->show(); m_findDlg->raise(); });
    connect(m_findDlg, &FindDialog::findNext, this, &MainWindow::findNext);
    QAction *repAct = new QAction("替换", this);
    connect(repAct, &QAction::triggered, this, [=]{ m_replaceDlg->show(); m_replaceDlg->raise(); });
    connect(m_replaceDlg, &ReplaceDialog::findNext, this, &MainWindow::doFind);
    connect(m_replaceDlg, &ReplaceDialog::replaceOne, this, &MainWindow::doReplaceOne);
    connect(m_replaceDlg, &ReplaceDialog::replaceAll, this, &MainWindow::doReplaceAll);

    // 视图
    QAction *tbAct = new QAction("工具栏", this); tbAct->setCheckable(true); tbAct->setChecked(true);
    QAction *sbAct = new QAction("状态栏", this); sbAct->setCheckable(true); sbAct->setChecked(true);
    QToolBar *tb = findChild<QToolBar*>();
    connect(tbAct, &QAction::toggled, tb, &QToolBar::setVisible);
    connect(sbAct, &QAction::toggled, this, [=](bool on){ statusBar()->setVisible(on); });

    // 帮助
    QAction *aboutAct = new QAction("关于", this);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    // 存储菜单引用
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction("新建"); fileMenu->addAction("打开");
    fileMenu->addAction(save); fileMenu->addAction(saveAs);
    fileMenu->addSeparator(); fileMenu->addAction(quit);

    QMenu *editMenu = menuBar()->addMenu("编辑(&E)");
    editMenu->addAction(undo); editMenu->addAction(redo);
    editMenu->addSeparator();
    editMenu->addAction(m_actCut); editMenu->addAction(m_actCopy); editMenu->addAction(paste);
    editMenu->addAction(selectAll);

    QMenu *formatMenu = menuBar()->addMenu("格式(&O)");
    formatMenu->addAction(fontAct); formatMenu->addAction(colorAct);
    formatMenu->addAction(bgAct); formatMenu->addAction(wrapAct);

    QMenu *searchMenu = menuBar()->addMenu("搜索(&S)");
    searchMenu->addAction(findAct); searchMenu->addAction(repAct);

    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    viewMenu->addAction(tbAct); viewMenu->addAction(sbAct);

    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction(aboutAct);

    // 编辑按钮使能
    connect(m_editor, &CodeEditor::copyAvailable, m_actCut, &QAction::setEnabled);
    connect(m_editor, &CodeEditor::copyAvailable, m_actCopy, &QAction::setEnabled);
    m_actCut->setEnabled(false); m_actCopy->setEnabled(false);
}

void MainWindow::createMenuToolBar()
{
    QToolBar *tb = addToolBar("主工具栏");
    tb->setMovable(false);
    // 把菜单里的Action按分组加到工具栏
    QList<QMenu*> menus = menuBar()->findChildren<QMenu*>();
    // 文件分组
    for (QAction *a : menus.at(0)->actions()) if(!a->isSeparator() && !a->text().isEmpty()) tb->addAction(a);
    tb->addSeparator();
    for (QAction *a : menus.at(1)->actions()) if(!a->isSeparator() && !a->text().isEmpty()) tb->addAction(a);
    tb->addSeparator();
    for (QAction *a : menus.at(3)->actions()) if(!a->isSeparator() && !a->text().isEmpty()) tb->addAction(a);
}

void MainWindow::newFile()
{
    if (maybeSave()) { m_editor->clear(); setCurrentFile(""); }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString path = QFileDialog::getOpenFileName(this, "打开文件", QString(),
                       "文本文件 (*.txt *.cpp *.h *.py);;所有文件 (*)");
        if (!path.isEmpty()) loadFile(path);
    }
}

void MainWindow::loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件：" + path);
        return;
    }
    QTextStream ts(&f);
    m_editor->setPlainText(ts.readAll());
    f.close();
    setCurrentFile(path);
    statusBar()->showMessage("已打开 " + path, 3000);
}

bool MainWindow::saveFile()
{
    if (m_curFile.isEmpty()) return saveAsFile();
    QFile f(m_curFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法写入文件：" + m_curFile);
        return false;
    }
    QTextStream ts(&f); ts << m_editor->toPlainText();
    f.close();
    m_editor->document()->setModified(false);
    updateWindowTitle();
    statusBar()->showMessage("已保存 " + m_curFile, 3000);
    return true;
}

bool MainWindow::saveAsFile()
{
    QString path = QFileDialog::getSaveFileName(this, "另存为", m_curFile,
                     "文本文件 (*.txt);;所有文件 (*)");
    if (path.isEmpty()) return false;
    m_curFile = path;
    return saveFile();
}

bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified()) return true;
    auto ret = QMessageBox::question(this, "未保存",
        "文档已修改，是否保存？", QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
    if (ret == QMessageBox::Save) return saveFile();
    if (ret == QMessageBox::Cancel) return false;
    return true;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave()) e->accept(); else e->ignore();
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_curFile = path;
    m_editor->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString name = m_curFile.isEmpty() ? "未命名.txt" : QFileInfo(m_curFile).fileName();
    setWindowTitle(QString("%1[*] - 简易文本编辑器").arg(name));
    setWindowModified(m_editor->document()->isModified());
}

void MainWindow::textModified() { updateWindowTitle(); }

void MainWindow::updateCursorPos()
{
    QTextCursor c = m_editor->textCursor();
    m_posLabel->setText(QString("行:%1 列:%2").arg(c.blockNumber()+1).arg(c.columnNumber()+1));
}

void MainWindow::findNext(const QString &text, bool cs)
{
    QTextDocument::FindFlags f = cs ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags();
    if (!m_editor->find(text, f)) {
        // 回到开头再找一次
        m_editor->moveCursor(QTextCursor::Start);
        if (!m_editor->find(text, f))
            QMessageBox::information(this, "查找", QString("已找不到 \"%1\"").arg(text));
    }
}

void MainWindow::doFind(const QString &text)
{
    if (!text.isEmpty()) findNext(text, false);
}

void MainWindow::doReplaceOne(const QString &find, const QString &replace)
{
    QTextCursor c = m_editor->textCursor();
    if (c.selectedText() == find) { c.insertText(replace); }
    findNext(find, false);
}

void MainWindow::doReplaceAll(const QString &find, const QString &replace)
{
    if (find.isEmpty()) return;
    m_editor->moveCursor(QTextCursor::Start);
    int n = 0;
    while (m_editor->find(find)) {
        m_editor->textCursor().insertText(replace);
        ++n;
    }
    QMessageBox::information(this, "替换", QString("共替换 %1 处").arg(n));
}

void MainWindow::about()
{
    QMessageBox::about(this, "关于",
        "<b>简易文本编辑器</b><br>Qt 应用实验作业<br>陈猛亿 2024414290201");
}
