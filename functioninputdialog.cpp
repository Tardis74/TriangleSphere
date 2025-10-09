#include "functioninputdialog.h"

FunctionInputDialog::FunctionInputDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Enter Position Functions");
    setMinimumSize(500, 400);

    QGridLayout *mainLayout = new QGridLayout(this);

    // Point 1
    QGroupBox *point1Group = new QGroupBox("Point 1 (P1)");
    QGridLayout *point1Layout = new QGridLayout(point1Group);
    point1Layout->addWidget(new QLabel("x₁(t) ="), 0, 0);
    x1Edit = new QLineEdit("50 + 20*cos(t)");
    point1Layout->addWidget(x1Edit, 0, 1);
    point1Layout->addWidget(new QLabel("y₁(t) ="), 1, 0);
    y1Edit = new QLineEdit("50 + 20*sin(t)");
    point1Layout->addWidget(y1Edit, 1, 1);
    mainLayout->addWidget(point1Group, 0, 0, 1, 2);

    // Point 2
    QGroupBox *point2Group = new QGroupBox("Point 2 (P2)");
    QGridLayout *point2Layout = new QGridLayout(point2Group);
    point2Layout->addWidget(new QLabel("x₂(t) ="), 0, 0);
    x2Edit = new QLineEdit("100 + 15*cos(2*t)");
    point2Layout->addWidget(x2Edit, 0, 1);
    point2Layout->addWidget(new QLabel("y₂(t) ="), 1, 0);
    y2Edit = new QLineEdit("50 + 15*sin(2*t)");
    point2Layout->addWidget(y2Edit, 1, 1);
    mainLayout->addWidget(point2Group, 1, 0, 1, 2);

    // Point 3
    QGroupBox *point3Group = new QGroupBox("Point 3 (P3)");
    QGridLayout *point3Layout = new QGridLayout(point3Group);
    point3Layout->addWidget(new QLabel("x₃(t) ="), 0, 0);
    x3Edit = new QLineEdit("75 + 25*cos(0.5*t)");
    point3Layout->addWidget(x3Edit, 0, 1);
    point3Layout->addWidget(new QLabel("y₃(t) ="), 1, 0);
    y3Edit = new QLineEdit("100 + 25*sin(0.5*t)");
    point3Layout->addWidget(y3Edit, 1, 1);
    mainLayout->addWidget(point3Group, 2, 0, 1, 2);

    // Info label
    QLabel *infoLabel = new QLabel("Use variable 't' for time. Supported functions: sin, cos, tan, exp, log, sqrt, etc.");
    infoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 5px; }");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel, 3, 0, 1, 2);

    // Buttons
    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");
    QPushButton *resetButton = new QPushButton("Reset to Default");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout, 4, 0, 1, 2);

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, [this]() {
        x1Edit->setText("50 + 20*cos(t)");
        y1Edit->setText("50 + 20*sin(t)");
        x2Edit->setText("100 + 15*cos(2*t)");
        y2Edit->setText("50 + 15*sin(2*t)");
        x3Edit->setText("75 + 25*cos(0.5*t)");
        y3Edit->setText("100 + 25*sin(0.5*t)");
    });
}

FunctionInputDialog::~FunctionInputDialog() {}

QString FunctionInputDialog::getX1() const { return x1Edit->text(); }
QString FunctionInputDialog::getY1() const { return y1Edit->text(); }
QString FunctionInputDialog::getX2() const { return x2Edit->text(); }
QString FunctionInputDialog::getY2() const { return y2Edit->text(); }
QString FunctionInputDialog::getX3() const { return x3Edit->text(); }
QString FunctionInputDialog::getY3() const { return y3Edit->text(); }

void FunctionInputDialog::setFunctions(const QStringList &functions) {
    if (functions.size() >= 6) {
        x1Edit->setText(functions[0]);
        y1Edit->setText(functions[1]);
        x2Edit->setText(functions[2]);
        y2Edit->setText(functions[3]);
        x3Edit->setText(functions[4]);
        y3Edit->setText(functions[5]);
    }
}
