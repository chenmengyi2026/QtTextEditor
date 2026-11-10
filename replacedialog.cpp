#include "replacedialog.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

ReplaceDialog::ReplaceDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("替换");
    setMinimumWidth(380);

    m_find = new QLineEdit(this);
    m_find->setPlaceholderText("请输入被替换的内容");
    m_replace = new QLineEdit(this);
    m_replace->setPlaceholderText("请输入替换后的内容");

    QPushButton *btnFind = new QPushButton("查找下一个", this);
    QPushButton *btnReplace = new QPushButton("替换", this);
    QPushButton *btnReplaceAll = new QPushButton("全部替换", this);
    QPushButton *btnClose = new QPushButton("关闭", this);

    QHBoxLayout *row1 = new QHBoxLayout;
    row1->addWidget(new QLabel("查找内容:", this));
    row1->addWidget(m_find);

    QHBoxLayout *row2 = new QHBoxLayout;
    row2->addWidget(new QLabel("替换为:", this));
    row2->addWidget(m_replace);

    QHBoxLayout *rowBtn = new QHBoxLayout;
    rowBtn->addWidget(btnFind);
    rowBtn->addWidget(btnReplace);
    rowBtn->addWidget(btnReplaceAll);
    rowBtn->addStretch();
    rowBtn->addWidget(btnClose);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(row1);
    main->addLayout(row2);
    main->addLayout(rowBtn);

    connect(btnFind, &QPushButton::clicked, this, [this]{ emit findNext(m_find->text()); });
    connect(btnReplace, &QPushButton::clicked, this, [this]{
        emit replaceOne(m_find->text(), m_replace->text());
    });
    connect(btnReplaceAll, &QPushButton::clicked, this, [this]{
        emit replaceAll(m_find->text(), m_replace->text());
    });
    connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
}

QString ReplaceDialog::findText() const
{
    return m_find->text();
}

QString ReplaceDialog::replaceText() const
{
    return m_replace->text();
}
