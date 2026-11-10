#include "finddialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

FindDialog::FindDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("查找");
    setMinimumWidth(320);

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText("请输入查找内容");
    m_case = new QCheckBox("区分大小写", this);

    QPushButton *btnFind = new QPushButton("查找下一个", this);
    btnFind->setDefault(true);
    QPushButton *btnClose = new QPushButton("关闭", this);

    QHBoxLayout *row1 = new QHBoxLayout;
    row1->addWidget(new QLabel("查找内容:", this));
    row1->addWidget(m_edit);

    QHBoxLayout *rowBtn = new QHBoxLayout;
    rowBtn->addStretch();
    rowBtn->addWidget(btnFind);
    rowBtn->addWidget(btnClose);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(row1);
    main->addWidget(m_case);
    main->addLayout(rowBtn);

    connect(btnFind, &QPushButton::clicked, this, &FindDialog::onFind);
    connect(m_edit, &QLineEdit::returnPressed, this, &FindDialog::onFind);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
}

QString FindDialog::text() const
{
    return m_edit->text();
}

bool FindDialog::caseSensitive() const
{
    return m_case->isChecked();
}

void FindDialog::onFind()
{
    emit findNext(m_edit->text(), m_case->isChecked());
}
