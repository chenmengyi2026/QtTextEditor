#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
class QLineEdit;

class ReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    QString findText() const;
    QString replaceText() const;
signals:
    void findNext(const QString &text);
    void replaceOne(const QString &find, const QString &replace);
    void replaceAll(const QString &find, const QString &replace);
private:
    QLineEdit *m_find;
    QLineEdit *m_replace;
};

#endif // REPLACEDIALOG_H
