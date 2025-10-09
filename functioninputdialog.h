#ifndef FUNCTIONINPUTDIALOG_H
#define FUNCTIONINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>

class FunctionInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionInputDialog(QWidget *parent = nullptr);
    ~FunctionInputDialog();

    QString getX1() const;
    QString getY1() const;
    QString getX2() const;
    QString getY2() const;
    QString getX3() const;
    QString getY3() const;

    void setFunctions(const QStringList &functions);

private:
    QLineEdit *x1Edit, *y1Edit, *x2Edit, *y2Edit, *x3Edit, *y3Edit;
};

#endif // FUNCTIONINPUTDIALOG_H
