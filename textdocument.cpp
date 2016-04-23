#include "textdocument.h"

#include <QFileInfo>
#include <QPlainTextDocumentLayout>
#include <QRegExp>
#include <QTextBlock>
#include <QTextStream>
#include <QUrl>

#include "cppsyntaxhighlighter.h"

namespace mote
{
    TextDocument::TextDocument( QObject* parent )
        : QTextDocument( parent ),
          m_readOnly( false ),
          m_generateBOM( false ),
          m_textCodec( QTextCodec::codecForLocale() ),
#ifdef Q_OS_WIN
          m_newlineChar( "\r\n" ),
#else
          m_newlineChar( "\n" ),
#endif
          m_syntaxHighlighter( NULL )
    {
        setDocumentLayout( new QPlainTextDocumentLayout( this ) );

        connect(
            this, SIGNAL( filePathChanged( TextDocument* ) ),
            SLOT( onFilePathChanged( void ) ) );
    }

    QString TextDocument::filePath( void )const
    {
        return QUrl( metaInformation( QTextDocument::DocumentUrl ) ).toLocalFile();
    }

    QString TextDocument::fileName( void )const
    {
        return QFileInfo( filePath() ).fileName();
    }

    QFont TextDocument::font( void )const
    {
        return defaultFont();
    }

    void TextDocument::setFont( const QFont& font )
    {
        setDefaultFont( font );
        emit fontChanged();
    }

    QString TextDocument::newlineCharacter( void )const
    {
        return m_newlineChar;
    }

    void TextDocument::setNewlineCharacter( const QString& newlineCharacter )
    {
        if( newlineCharacter != m_newlineChar )
        {
            m_newlineChar = newlineCharacter;
            setModified( true );
            emit newlineCharacterChanged( this );
        }
    }

    QTextCodec* TextDocument::textCodec( void )const
    {
        return m_textCodec;
    }

    void TextDocument::setTextCodec( QTextCodec* codec )
    {
        if( !codec || ( codec == m_textCodec ) )
        {
            return;
        }

        if( isModified() )
        {
            m_textCodec = codec;
        }
        else
        {
            setPlainText( codec->toUnicode( m_data ) );
            setModified( false );
            m_textCodec = codec;
        }

        emit textCodecChanged( this );
    }

    bool TextDocument::generateByteOrderMark( void )const
    {
        return m_generateBOM;
    }

    void TextDocument::setGenerateByteOrderMark( const bool onoff )
    {
        if( ( onoff && !m_generateBOM ) || ( !onoff && m_generateBOM ) )
        {
            m_generateBOM = onoff;
            setModified( true );
            emit generateByteOrderMarkChanged( this );
        }
    }

    bool TextDocument::openFile( const QString& path )
    {
        bool readOnly = false;
        QFile file( path );
        if( !file.open( QIODevice::ReadWrite ) )
        {
            if( !file.open( QIODevice::ReadOnly ) )
            {
                return false;
            }
            else
            {
                readOnly = true;
            }
        }

        QByteArray data = file.readAll();
        QByteArray bom;
        QTextCodec* codec = detectCodec( data, bom );
        if( !codec )
        {
            return false;
        }

        bool bomExists = false;
        if( !bom.isEmpty() )
        {
            data.remove( 0, bom.length() );
            bomExists = true;
        }

        const QString str = codec->toUnicode( data );
        const QString newlineChar = detectNewlineCharacter( str );

        const QString oldFilePath = filePath();

        m_data = data;
        m_readOnly = readOnly;
        m_textCodec = codec;
        m_newlineChar = newlineChar;
        setMetaInformation( QTextDocument::DocumentUrl, QUrl::fromLocalFile( path ).toString() );
        m_generateBOM = bomExists;

        setPlainText( str );
        setModified( false );
        clearUndoRedoStacks();

        emit textCodecChanged( this );
        if( filePath() != oldFilePath )
        {
            emit filePathChanged( this );
        }
        emit newlineCharacterChanged( this );
        emit generateByteOrderMarkChanged( this );

        return true;
    }

    bool TextDocument::saveFile( const QString& path )
    {
        QString str;
        QTextBlock block = begin();
        if( block.isValid() )
        {
            while( true )
            {
                str += block.text();
                block = block.next();
                if( block.isValid() )
                {
                    str += m_newlineChar;
                }
                else
                {
                    break;
                }
            }
        }

        const QByteArray data = m_textCodec->fromUnicode( str );

        QFile file( path );
        if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
            return false;
        }

        if( generateByteOrderMark() )
        {
            writeBOM( file );
        }

        file.write( data );

        m_data = data;

        if( path != filePath() )
        {
            setMetaInformation( QTextDocument::DocumentUrl, QUrl::fromLocalFile( path ).toString() );
            emit filePathChanged( this );
        }

        setModified( false );

        return true;
    }

    bool TextDocument::reload( void )
    {
        const QString path = filePath();
        if( path.isEmpty() )
        {
            return false;
        }

        return openFile( path );
    }

    QTextCodec* TextDocument::detectCodec(
        const QByteArray& data,
        QByteArray& bom )const
    {
        const char utf8_bom[] = { ( char )0xEF, ( char )0xBB, ( char )0xBF };
        if( data.startsWith( QByteArray( utf8_bom, 3 ) ) )
        {
            bom = QByteArray( utf8_bom, 3 );
            return QTextCodec::codecForName( "UTF-8" );
        }

        const char utf16le_bom[] = { ( char )0xFF, ( char )0xFE };
        if( data.startsWith( QByteArray( utf16le_bom, 2 ) ) )
        {
            bom = QByteArray( utf16le_bom, 2 );
            return QTextCodec::codecForName( "UTF-16LE" );
        }

        const char utf16be_bom[] = { ( char )0xFE, ( char )0xFF };
        if( data.startsWith( QByteArray( utf16be_bom, 2 ) ) )
        {
            bom = QByteArray( utf16be_bom, 2 );
            return QTextCodec::codecForName( "UTF-16BE" );
        }

        const char utf32le_bom[] = { ( char )0xFF, ( char )0xFE, ( char )0x00, ( char )0x00 };
        if( data.startsWith( QByteArray( utf32le_bom, 4 ) ) )
        {
            bom = QByteArray( utf32le_bom, 4 );
            return QTextCodec::codecForName( "UTF-32LE" );
        }

        const char utf32be_bom[] = { ( char )0x00, ( char )0x00, ( char )0xFE, ( char )0xFF };
        if( data.startsWith( QByteArray( utf32be_bom, 4 ) ) )
        {
            bom = QByteArray( utf32be_bom, 4 );
            return QTextCodec::codecForName( "UTF-32BE" );
        }

        // FIXME
        /*
        QList<QTextCodec*> codecs;
        codecs.push_back( QTextCodec::codecForLocale() );
        codecs.push_back( QTextCodec::codecForName( "UTF-8" ) );
        for( int i = 0; i < codecs.size(); ++i )
        {
            QTextCodec* codec = codecs.at( i );
            if( codec )
            {
                QTextCodec::ConverterState state;
                codec->toUnicode( data.constData(), data.length(), &state );
                if( state.invalidChars == 0 )
                {
                    return codec;
                }
            }
        }
        */

        return QTextCodec::codecForLocale();
    }

    QString TextDocument::detectNewlineCharacter( const QString& str )const
    {
        QString newlineChar;
        const int index = str.indexOf( QRegExp( "\r|\n" ) );
        if( index >= 0 )
        {
            newlineChar = str[index];
            if( ( str[index] == '\r' ) &&
                ( str.length() > index + 1 ) &&
                ( str[index + 1] == '\n' ) )
            {
                newlineChar += '\n';
            }
        }
        else
        {
#ifdef Q_OS_WIN
            newlineChar = "\r\n";
#else
            newlineChar = "\n";
#endif
        }
        return newlineChar;
    }

    void TextDocument::writeBOM( QFile& file )const
    {
        const QString name = m_textCodec->name().constData();
        if( name == "UTF-8" )
        {
            const char utf8_bom[] = { ( char )0xEF, ( char )0xBB, ( char )0xBF };
            file.write( utf8_bom, 3 );
        }
        else if( ( name == "UTF-16" ) || ( name == "UTF-16LE" ) )
        {
            const char utf16le_bom[] = { ( char )0xFF, ( char )0xFE };
            file.write( utf16le_bom, 2 );
        }
        else if( name == "UTF-16BE" )
        {
            const char utf16be_bom[] = { ( char )0xFE, ( char )0xFF };
            file.write( utf16be_bom, 2 );
        }
        else if( ( name == "UTF-32" ) || ( name == "UTF-32LE" ) )
        {
            const char utf32le_bom[] = { ( char )0xFF, ( char )0xFE, ( char )0x00, ( char )0x00 };
            file.write( utf32le_bom, 4 );
        }
        else if( name == "UTF-32BE" )
        {
            const char utf32be_bom[] = { ( char )0x00, ( char )0x00, ( char )0xFE, ( char )0xFF };
            file.write( utf32be_bom, 4 );
        }
    }

    void TextDocument::onFilePathChanged( void )
    {
        delete m_syntaxHighlighter;
        m_syntaxHighlighter = NULL;

        const QString filePath = this->filePath();
        if( filePath.endsWith( ".cpp" ) ||
            filePath.endsWith( ".cc" ) ||
            filePath.endsWith( ".c" ) ||
            filePath.endsWith( ".h" ) ||
            filePath.endsWith( ".hpp" ) ||
            filePath.endsWith( ".inl" ) )
        {
            m_syntaxHighlighter = new CppSyntaxHighlighter( this );
        }
    }
}
