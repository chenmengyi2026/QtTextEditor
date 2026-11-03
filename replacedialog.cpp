#include "replacedialog.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

ReplaceDialog::ReplaceDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("替换");
    m_find = new QLineEdit(this);
    m_replace = new QLineEdit(this);
    QPushButton *btnFind = new QPushButton("查找下一个", this);
    QPushButton *btnRep = new QPushButton("替换", this);
    QPushButton *btnAll = new QPushButton("全部替换", this);

    QVBoxLayout *main = new QVBoxLayout(this);
    QHBoxLayout *l1 = new QHBoxLayout; l1->addWidget(new QLabel("查找:", this)); l1->addWidget(m_find);
    QHBoxLayout *l2 = new QHBoxLayout; l2->addWidget(new QLabel("替换为:", this)); l2->addWidget(m_replace);
    QHBoxLayout *l3 = new QHBoxLayout; l3->addWidget(btnFind); l3->addWidget(btnRep); l3->addWidget(btnAll);
    main->addLayout(l1); main->addLayout(l2); main->addLayout(l3);

    connect(btnFind, &QPushButton::clicked, this, [=]{ emit findNext(m_find->text()); });
    connect(btnRep, &QPushButton::clicked, this, [=]{ emit replaceOne(m_find->text(), m_replace->text()); });
    connect(btnAll, &QPushButton::clicked, this, [=]{ emit replaceAll(m_find->text(), m_replace->text()); });
    setMinimumWidth(340);
}

QString ReplaceDialog::findText() const { return m_find->text(); }
QString ReplaceDialog::replaceText() const { return m_replace->text(); }
