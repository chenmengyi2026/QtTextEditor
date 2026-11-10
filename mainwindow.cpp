#include "mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDialog>
#include <QColorDialog>
#include <QStyle>
#include <QLabel>
#include <QCloseEvent>
#include <QTextCursor>
#include <QFile>
#include <QFileInfo>
#include <QWidget>
#include <QHBoxLayout>
#include <QTextDocument>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(0);

    m_editor = new CodeEditor(central);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    layout->addWidget(m_editor);
    setCentralWidget(central);

    m_findDlg = new FindDialog(this);
    m_replaceDlg = new ReplaceDialog(this);
    m_textColor = QColor(Qt::black);
    m_bgColor = QColor(Qt::white);

    createActions();
    createMenus();
    createToolBar();

    m_posLabel = new QLabel("行:1  列:1", this);
    m_infoLabel = new QLabel("字符数:0", this);
    statusBar()->addPermanentWidget(m_infoLabel);
    statusBar()->addPermanentWidget(m_posLabel);
    statusBar()->showMessage("就绪");

    connect(m_editor, &CodeEditor::textChanged, this, &MainWindow::updateStatusInfo);
    connect(m_editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateCursorPos);
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);

    applyColors();
    setCurrentFile(QString());
    resize(860, 600);
}

void MainWindow::createActions()
{
    QStyle *st = style();

    m_actNew = new QAction(st->standardIcon(QStyle::SP_FileIcon), "新建(&N)", this);
    m_actNew->setShortcut(QKeySequence::New);
    connect(m_actNew, &QAction::triggered, this, &MainWindow::newFile);

    m_actOpen = new QAction(st->standardIcon(QStyle::SP_DirOpenIcon), "打开(&O)", this);
    m_actOpen->setShortcut(QKeySequence::Open);
    connect(m_actOpen, &QAction::triggered, this, &MainWindow::openFile);

    m_actSave = new QAction(st->standardIcon(QStyle::SP_DialogSaveButton), "保存(&S)", this);
    m_actSave->setShortcut(QKeySequence::Save);
    connect(m_actSave, &QAction::triggered, this, &MainWindow::saveFile);

    m_actSaveAs = new QAction("另存为(&A)", this);
    m_actSaveAs->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(m_actSaveAs, &QAction::triggered, this, &MainWindow::saveAsFile);

    m_actQuit = new QAction("退出(&X)", this);
    m_actQuit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(m_actQuit, &QAction::triggered, this, &QWidget::close);

    m_actUndo = new QAction(st->standardIcon(QStyle::SP_ArrowBack), "撤销(&U)", this);
    m_actUndo->setShortcut(QKeySequence::Undo);
    connect(m_actUndo, &QAction::triggered, m_editor, &CodeEditor::undo);

    m_actRedo = new QAction(st->standardIcon(QStyle::SP_ArrowForward), "重做(&R)", this);
    m_actRedo->setShortcut(QKeySequence("Ctrl+Y"));
    connect(m_actRedo, &QAction::triggered, m_editor, &CodeEditor::redo);

    m_actCut = new QAction("剪切(&T)", this);
    m_actCut->setShortcut(QKeySequence::Cut);
    connect(m_actCut, &QAction::triggered, m_editor, &CodeEditor::cut);

    m_actCopy = new QAction("复制(&C)", this);
    m_actCopy->setShortcut(QKeySequence::Copy);
    connect(m_actCopy, &QAction::triggered, m_editor, &CodeEditor::copy);

    m_actPaste = new QAction("粘贴(&P)", this);
    m_actPaste->setShortcut(QKeySequence::Paste);
    connect(m_actPaste, &QAction::triggered, m_editor, &CodeEditor::paste);

    m_actSelectAll = new QAction("全选(&A)", this);
    m_actSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(m_actSelectAll, &QAction::triggered, m_editor, &CodeEditor::selectAll);

    m_actFont = new QAction("字体(&F)", this);
    connect(m_actFont, &QAction::triggered, this, [this]{
        bool ok = false;
        QFont font = QFontDialog::getFont(&ok, m_editor->font(), this, "选择字体");
        if (ok)
            m_editor->setFont(font);
    });

    m_actTextColor = new QAction("字体颜色(&C)", this);
    connect(m_actTextColor, &QAction::triggered, this, [this]{
        QColor color = QColorDialog::getColor(m_textColor, this, "选择字体颜色");
        if (color.isValid()) {
            m_textColor = color;
            applyColors();
        }
    });

    m_actBgColor = new QAction("背景颜色(&B)", this);
    connect(m_actBgColor, &QAction::triggered, this, [this]{
        QColor color = QColorDialog::getColor(m_bgColor, this, "选择背景颜色");
        if (color.isValid()) {
            m_bgColor = color;
            applyColors();
        }
    });

    m_actAutoWrap = new QAction("自动换行(&W)", this);
    m_actAutoWrap->setCheckable(true);
    m_actAutoWrap->setChecked(true);
    connect(m_actAutoWrap, &QAction::toggled, this, [this](bool checked){
        m_editor->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);
    });

    m_actLineNo = new QAction("显示行号(&L)", this);
    m_actLineNo->setCheckable(true);
    m_actLineNo->setChecked(true);
    connect(m_actLineNo, &QAction::toggled, m_editor, &CodeEditor::setLineNumbersVisible);

    m_actFind = new QAction("查找(&F)", this);
    m_actFind->setShortcut(QKeySequence::Find);
    connect(m_actFind, &QAction::triggered, this, [this]{
        m_findDlg->show();
        m_findDlg->raise();
        m_findDlg->activateWindow();
    });
    connect(m_findDlg, &FindDialog::findNext, this, &MainWindow::findNext);

    m_actReplace = new QAction("替换(&R)", this);
    m_actReplace->setShortcut(QKeySequence("Ctrl+H"));
    connect(m_actReplace, &QAction::triggered, this, [this]{
        m_replaceDlg->show();
        m_replaceDlg->raise();
        m_replaceDlg->activateWindow();
    });
    connect(m_replaceDlg, &ReplaceDialog::findNext, this, &MainWindow::doFind);
    connect(m_replaceDlg, &ReplaceDialog::replaceOne, this, &MainWindow::doReplaceOne);
    connect(m_replaceDlg, &ReplaceDialog::replaceAll, this, &MainWindow::doReplaceAll);

    m_actToolBar = new QAction("工具栏(&T)", this);
    m_actToolBar->setCheckable(true);
    m_actToolBar->setChecked(true);

    m_actStatusBar = new QAction("状态栏(&S)", this);
    m_actStatusBar->setCheckable(true);
    m_actStatusBar->setChecked(true);
    connect(m_actStatusBar, &QAction::toggled, this, [this](bool checked){
        statusBar()->setVisible(checked);
    });

    m_actAbout = new QAction("关于(&A)", this);
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::about);

    // 剪切、复制只在有选中文本时可用，撤销、重做随文档历史变化
    connect(m_editor, &CodeEditor::copyAvailable, m_actCut, &QAction::setEnabled);
    connect(m_editor, &CodeEditor::copyAvailable, m_actCopy, &QAction::setEnabled);
    connect(m_editor, &CodeEditor::undoAvailable, m_actUndo, &QAction::setEnabled);
    connect(m_editor, &CodeEditor::redoAvailable, m_actRedo, &QAction::setEnabled);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this]{
        m_actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    });

    m_actCut->setEnabled(false);
    m_actCopy->setEnabled(false);
    m_actUndo->setEnabled(false);
    m_actRedo->setEnabled(false);
    m_actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction(m_actNew);
    fileMenu->addAction(m_actOpen);
    fileMenu->addAction(m_actSave);
    fileMenu->addAction(m_actSaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actQuit);

    QMenu *editMenu = menuBar()->addMenu("编辑(&E)");
    editMenu->addAction(m_actUndo);
    editMenu->addAction(m_actRedo);
    editMenu->addSeparator();
    editMenu->addAction(m_actCut);
    editMenu->addAction(m_actCopy);
    editMenu->addAction(m_actPaste);
    editMenu->addAction(m_actSelectAll);

    QMenu *formatMenu = menuBar()->addMenu("格式(&O)");
    formatMenu->addAction(m_actFont);
    formatMenu->addAction(m_actTextColor);
    formatMenu->addAction(m_actBgColor);
    formatMenu->addSeparator();
    formatMenu->addAction(m_actAutoWrap);

    QMenu *searchMenu = menuBar()->addMenu("搜索(&S)");
    searchMenu->addAction(m_actFind);
    searchMenu->addAction(m_actReplace);

    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    viewMenu->addAction(m_actToolBar);
    viewMenu->addAction(m_actStatusBar);
    viewMenu->addSeparator();
    viewMenu->addAction(m_actLineNo);

    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction(m_actAbout);
}

void MainWindow::createToolBar()
{
    m_toolBar = addToolBar("主工具栏");
    m_toolBar->setMovable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // 文件组
    m_toolBar->addAction(m_actNew);
    m_toolBar->addAction(m_actOpen);
    m_toolBar->addAction(m_actSave);
    m_toolBar->addAction(m_actSaveAs);
    m_toolBar->addSeparator();
    // 编辑组
    m_toolBar->addAction(m_actUndo);
    m_toolBar->addAction(m_actRedo);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_actCut);
    m_toolBar->addAction(m_actCopy);
    m_toolBar->addAction(m_actPaste);
    m_toolBar->addSeparator();
    // 搜索组
    m_toolBar->addAction(m_actFind);
    m_toolBar->addAction(m_actReplace);

    connect(m_actToolBar, &QAction::toggled, m_toolBar, &QToolBar::setVisible);
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        m_editor->clear();
        setCurrentFile(QString());
        statusBar()->showMessage("已新建空白文档", 2000);
    }
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    QString path = QFileDialog::getOpenFileName(this, "打开文件", QString(),
                   "文本文件 (*.txt *.cpp *.h);;所有文件 (*.*)");
    if (!path.isEmpty())
        loadFile(path);
}

void MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "打开失败",
                             QString("无法读取文件：%1\n请确认文件存在、且没有被其他程序占用。")
                             .arg(QFileInfo(path).fileName()));
        return;
    }
    QTextStream in(&file);
    m_editor->setPlainText(in.readAll());
    file.close();

    setCurrentFile(path);
    statusBar()->showMessage(QString("已打开 %1").arg(QFileInfo(path).fileName()), 3000);
}

bool MainWindow::saveFile()
{
    if (m_curFile.isEmpty())
        return saveAsFile();

    QFile file(m_curFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "保存失败",
                             QString("无法写入文件：%1\n请检查该文件是否被设成只读，或者所在目录是否可写。")
                             .arg(QFileInfo(m_curFile).fileName()));
        return false;
    }
    QTextStream out(&file);
    out << m_editor->toPlainText();
    file.close();

    // 只有真正写盘成功，才清除“已修改”标记
    m_editor->document()->setModified(false);
    updateWindowTitle();
    statusBar()->showMessage(QString("已保存 %1").arg(QFileInfo(m_curFile).fileName()), 3000);
    return true;
}

bool MainWindow::saveAsFile()
{
    QString path = QFileDialog::getSaveFileName(this, "另存为",
                   m_curFile.isEmpty() ? "未命名.txt" : m_curFile,
                   "文本文件 (*.txt);;所有文件 (*.*)");
    if (path.isEmpty())
        return false;

    m_curFile = path;
    return saveFile();
}

bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified())
        return true;

    QMessageBox::StandardButton ret = QMessageBox::question(
        this, "未保存的修改",
        "文档已经修改，是否保存？",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_curFile = path;
    m_editor->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString name = m_curFile.isEmpty() ? QString("未命名.txt")
                                       : QFileInfo(m_curFile).fileName();
    setWindowTitle(QString("%1[*] - 简易文本编辑器").arg(name));
    setWindowModified(m_editor->document()->isModified());
}

void MainWindow::updateStatusInfo()
{
    QString text = m_editor->toPlainText();
    m_infoLabel->setText(QString("字符数:%1  行数:%2")
                         .arg(text.length())
                         .arg(m_editor->blockCount()));
}

void MainWindow::updateCursorPos()
{
    QTextCursor cursor = m_editor->textCursor();
    m_posLabel->setText(QString("行:%1  列:%2")
                        .arg(cursor.blockNumber() + 1)
                        .arg(cursor.columnNumber() + 1));
}

void MainWindow::applyColors()
{
    m_editor->setStyleSheet(
        QString("QPlainTextEdit{color:%1;background-color:%2;}")
        .arg(m_textColor.name())
        .arg(m_bgColor.name()));
}

void MainWindow::findNext(const QString &text, bool caseSensitive)
{
    if (text.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    if (m_editor->find(text, flags))
        return;

    // 到文末还没找到，回到开头再找一遍，仍然没有才提示
    m_editor->moveCursor(QTextCursor::Start);
    if (!m_editor->find(text, flags)) {
        QMessageBox::information(this, "查找",
                                 QString("全文已找不到“%1”。").arg(text));
    }
}

void MainWindow::doFind(const QString &text)
{
    findNext(text, false);
}

void MainWindow::doReplaceOne(const QString &find, const QString &replace)
{
    if (find.isEmpty())
        return;

    QTextCursor cursor = m_editor->textCursor();
    if (cursor.selectedText() == find)
        cursor.insertText(replace);

    findNext(find, false);
}

void MainWindow::doReplaceAll(const QString &find, const QString &replace)
{
    if (find.isEmpty())
        return;

    m_editor->moveCursor(QTextCursor::Start);
    int count = 0;
    while (m_editor->find(find)) {
        m_editor->textCursor().insertText(replace);
        ++count;
    }
    QMessageBox::information(this, "替换",
                             QString("共替换 %1 处。").arg(count));
}

void MainWindow::about()
{
    QMessageBox::about(this, "关于",
        QString("<h3>简易文本编辑器</h3>"
                "<p>Qt 应用程序开发 实验二</p>"
                "<p>作者：陈猛亿　学号：2024414290201</p>"
                "<p>班级：2024软件工程2班</p>"
                "<p>版本：V1.0</p>"));
}
