#include <QtCore>
#include <QDebug>

#include "internal/main.h"
#include "internal/v4l2.h"
#include "internal/codecengine.h"


int main(int _argc, char* _argv[])
{
    QCoreApplication a(_argc, _argv);

    return a.exec();
}
