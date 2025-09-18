#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <csignal>
#include <cstdlib>

void signalHandler(int signal)
{
    qCritical() << "Received signal:" << signal;
    QApplication::quit();
    std::exit(signal);
}

int main(int argc, char* argv[]) {
    // Установка обработчиков сигналов
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE, signalHandler);

    try {
        QApplication app(argc, argv);
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    }
    catch (const std::exception& e) {
        qCritical() << "Exception caught in main: " << e.what();
        return 1;
    }
    catch (...) {
        qCritical() << "Unknown exception caught in main";
        return 1;
    }
}
