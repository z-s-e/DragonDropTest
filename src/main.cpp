#include "dragsource.h"
#include "droparea.h"
#include "widget.h"
#include <QApplication>

#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.setWindowTitle("DragonDropTest " DRAGONDROPTEST_VERSION_STRING);
    w.show();

    return a.exec();
}
