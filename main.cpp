#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QTranslator>

#include "documentsystem.h"
#include "mainwindow.h"
#include "settings.h"
#include "textdocument.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QTranslator translator;
    QDir dir( QCoreApplication::applicationDirPath() );
#ifdef Q_OS_MAC
    dir.cdUp();
    if( dir.cd( "Resources" ) )
    {
        if( !translator.load( QLocale::system(), "mote", "_", dir.path() ) )
        {
            dir.cdUp();
            dir.cdUp();
            dir.cdUp();
            translator.load( QLocale::system(), "mote", "_", dir.path() );
        }
    }
    else
    {
        dir.cdUp();
        dir.cdUp();
        translator.load( QLocale::system(), "mote", "_", dir.path() );
    }
#else
    translator.load( QLocale::system(), "mote", "_", dir.path() );
#endif
    QCoreApplication::installTranslator( &translator );

    mote::Settings settings;

    mote::DocumentSystem documentSystem( &settings );

    {
        mote::MainWindow* win = new mote::MainWindow( &settings, &documentSystem );
        win->setAttribute( Qt::WA_DeleteOnClose );
        win->restoreGeometryAndState();
        win->show();
    }

    const int ret = app.exec();

    return ret;
}
