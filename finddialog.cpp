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
    m_edit = new QLineEdit(this);
    m_case = new QCheckBox("区分大小写", this);
    QPushButton *btn = new QPushButton("查找下一个", this);
    QHBoxLayout *l1 = new QHBoxLayout;
    l1->addWidget(new QLabel("查找内容:", this));
    l1->addWidget(m_edit);
    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(l1);
    main->addWidget(m_case);
    main->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &FindDialog::onFind);
    connect(m_edit, &QLineEdit::returnPressed, this, &FindDialog::onFind);
    setMinimumWidth(320);
}

QString FindDialog::text() const { return m_edit->text(); }
bool FindDialog::caseSensitive() const { return m_case->isChecked(); }

void FindDialog::onFind() { emit findNext(m_edit->text(), m_case->isChecked()); }
