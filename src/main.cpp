
#include <iostream>
#include <filesystem>
#include <string>

#include <QApplication>
#include <QHBoxLayout>

int main(int argc, char** argv)
{
    std::string currentLocalPath = std::getenv("HOME");

    QApplication app(argc, argv);

    return app.exec();
}
