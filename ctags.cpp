#include "ctags.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrl>

namespace mote
{
    CTags::CTags( void )
    {
    }

    bool CTags::exec( const QTextDocument* document )
    {
        if( !document )
        {
            return false;
        }

        const QString program = findCTags();
        if( program.isEmpty() )
        {
            return false;
        }

        QString fileName =
            QFileInfo( QUrl( document->metaInformation( QTextDocument::DocumentUrl ) ).toLocalFile() ).fileName();
        if( fileName.isEmpty() )
        {
            fileName = "temp.cpp";
        }

        QTemporaryFile tempFile(
            QDir( QDir::tempPath() ).filePath( QString( "XXXXXX_%1" ).arg( fileName ) ) );
        if( !tempFile.open() )
        {
            return false;
        }
        tempFile.write( document->toPlainText().toUtf8() );
        tempFile.close();

        QTemporaryFile tagsFile;
        if( !tagsFile.open() )
        {
            return false;
        }
        tagsFile.close();

        QStringList args;
        args += "--fields=Ksz";
        args += "-n";
        args += "-u";
        args += "-f";
        args += tagsFile.fileName();
        args += tempFile.fileName();

        const int ret = QProcess::execute( program, args );
        if( ret == -2 )
        {
            // ctags not exist
            return false;
        }

        return readTags( tagsFile.fileName() );
    }

    QString CTags::Entry::scope( void )const
    {
        for( int i = 0; i < extensionFields.size(); ++i )
        {
            const QString& field = extensionFields.at( i );
            if( field.startsWith( "file:" ) )
            {
                // Global
                break;
            }
            else if( field.startsWith( "class:" ) )
            {
                return field.mid( 6 );
            }
            else if( field.startsWith( "enum:" ) )
            {
                return field.mid( 5 );
            }
            else if( field.startsWith( "function:" ) )
            {
                return field.mid( 9 );
            }
            else if( field.startsWith( "struct:" ) )
            {
                return field.mid( 7 );
            }
            else if( field.startsWith( "union:" ) )
            {
                return field.mid( 6 );
            }
        }

        return QString();
    }

    QString CTags::Entry::kind( void )const
    {
        for( int i = 0; i < extensionFields.size(); ++i )
        {
            const QString& field = extensionFields.at( i );
            if( field.startsWith( "kind:" ) )
            {
                return field.mid( 5 );
            }
        }

        return QString();
    }

    int CTags::count( void )const
    {
        return m_entries.size();
    }

    const CTags::Entry& CTags::entry( int index )const
    {
        return m_entries.at( index );
    }

    QString CTags::findCTags( void )const
    {
#ifdef Q_OS_MAC
        // mote.app/Contents/MacOS/mote
        // mote.app/Contents/Frameworks/ctags
        // ctags
        QDir dir( QCoreApplication::applicationDirPath() );
        dir.cdUp();
        if( dir.cd( "Frameworks" ) )
        {
            if( dir.exists( "ctags" ) )
            {
                return dir.filePath( "ctags" );
            }
            dir.cdUp();
        }
        dir.cdUp();
        dir.cdUp();
        return dir.filePath( "ctags" );
#else
        // mote(.exe)
        // ctags(.exe)
        return QDir( QCoreApplication::applicationDirPath() ).filePath( "ctags" );
#endif
    }

    bool CTags::readTags( const QString& path )
    {
        QFile file( path );
        if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            return false;
        }

        m_entries.clear();

        QTextStream stream( &file );
        QRegExp regExp( "([^\\t]+)\\t+([^\\t]+)\\t+([^\\t]+);\"\\t+(.*)" );
        while( !stream.atEnd() )
        {
            const QString line = stream.readLine();

            if( line.startsWith( "!" ) )
            {
                continue;
            }

            if( !regExp.exactMatch( line ) )
            {
                qWarning( qPrintable( QString( "ctags: unexpected data: %1" ).arg( line ) ) );
                continue;
            }

            Entry entry;
            entry.tagName = regExp.cap( 1 );
            entry.fileName = regExp.cap( 2 );
            entry.exCmd = regExp.cap( 3 );
            entry.extensionFields = regExp.cap( 4 ).split( '\t' );
            m_entries.push_back( entry );
        }

        return true;
    }
}
