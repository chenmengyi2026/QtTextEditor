#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
class QLineEdit;
class QCheckBox;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = nullptr);
    QString text() const;
    bool caseSensitive() const;
signals:
    void findNext(const QString &text, bool cs);
private slots:
    void onFind();
private:
    QLineEdit *m_edit;
    QCheckBox *m_case;
};

#endif // FINDDIALOG_H
