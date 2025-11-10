#include "functioninputdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

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
    QLabel *infoLabel = new QLabel("Use variable 't' for time. Supported functions: sin, cos, tan, exp, log, sqrt. Example: 50 + 20*cos(t)");
    infoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 5px; }");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel, 3, 0, 1, 2);

    // Buttons
    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");
    QPushButton *resetButton = new QPushButton("Reset to Default");
    importButton = new QPushButton("Import from File"); // Новая кнопка

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(importButton); // Добавляем кнопку импорта
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
    connect(importButton, &QPushButton::clicked, this, &FunctionInputDialog::importFromFile);
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

void FunctionInputDialog::importFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Import Functions from File",
                                                    "",
                                                    "Text Files (*.txt);;All Files (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Import Error",
                             "Cannot open file: " + fileName);
        return;
    }

    QTextStream in(&file);
    QStringList functions;

    // Читаем файл и ищем функции
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Пропускаем пустые строки и комментарии
        if (line.isEmpty() || line.startsWith("#") || line.startsWith("//")) {
            continue;
        }

        // Пытаемся найти функции в формате: x1(t) = ... или просто x1 = ...
        if (line.contains("x1") && line.contains("=")) {
            functions.append(extractFunction(line));
        } else if (line.contains("y1") && line.contains("=")) {
            functions.append(extractFunction(line));
        } else if (line.contains("x2") && line.contains("=")) {
            functions.append(extractFunction(line));
        } else if (line.contains("y2") && line.contains("=")) {
            functions.append(extractFunction(line));
        } else if (line.contains("x3") && line.contains("=")) {
            functions.append(extractFunction(line));
        } else if (line.contains("y3") && line.contains("=")) {
            functions.append(extractFunction(line));
        }
    }

    file.close();

    if (functions.size() >= 6) {
        x1Edit->setText(functions[0]);
        y1Edit->setText(functions[1]);
        x2Edit->setText(functions[2]);
        y2Edit->setText(functions[3]);
        x3Edit->setText(functions[4]);
        y3Edit->setText(functions[5]);
        QMessageBox::information(this, "Import Successful",
                                 "Functions imported successfully from: " + fileName);
    } else {
        QMessageBox::warning(this, "Import Error",
                             "Could not find all required functions in the file.\n"
                             "Required functions: x1, y1, x2, y2, x3, y3");
    }
}

QString FunctionInputDialog::extractFunction(const QString& line)
{
    // Извлекаем выражение после знака '='
    int equalsPos = line.indexOf('=');
    if (equalsPos == -1) return "";

    QString expression = line.mid(equalsPos + 1).trimmed();

    // Убираем возможные точки с запятой в конце
    if (expression.endsWith(';')) {
        expression.chop(1);
    }

    return expression.trimmed();
}
