#include "textedit.h"

#ifdef Q_OS_WIN
#define STDCALL __stdcall
#else
#define STDCALL
#endif

extern "C"
{
    char* STDCALL AStyleMain(
        const char* textIn,
        const char* options,
        void( STDCALL *errorHandler )( int, char* ),
        char*( STDCALL *memoryAlloc )( unsigned long ) );

}

namespace mote
{
    static void STDCALL errorHandler( int /*errorNumber*/, char* /*errorMessage*/ )
    {
    }

    static char* STDCALL memoryAlloc( unsigned long memoryNeeded )
    {
        return new char[memoryNeeded];
    }

    static int calcIndent( const int tab_num, const QString& text )
    {
        if( tab_num == 0 )
        {
            return 0;
        }

        int column = 0;
        for( int i = 0; i < text.length() ; ++i )
        {
            const QChar ch = text[i];
            if ( ch == ' ' )
            {
                ++column;
            }
            else if ( ch == '\t' )
            {
                column = ( ( column / tab_num ) + 1 ) * tab_num;
            }
            else
            {
                break;
            }
        }

        return column / tab_num;
    }

    void TextEdit::formatSourceCode( void )
    {
        QTextCursor textCursor = this->textCursor();
        if( !textCursor.hasSelection() )
        {
            return;
        }

        QString option = "mode=c indent-namespaces pad-oper pad-paren-in convert-tabs style=break indent-cases min-conditional-indent=0 max-instatement-indent=70";
        option += QString( " indent=spaces=%1" ).arg( tabStopWidthBySpace() );

        QString text = textCursor.selectedText();
        text.replace( QChar( 0x2029 ), '\n' );

        char* textOut = AStyleMain(
                            text.toUtf8().constData(),
                            option.toUtf8().constData(),
                            errorHandler,
                            memoryAlloc );
        QString formattedText = QString::fromUtf8( textOut );
        delete[] textOut;
        textOut = NULL;

        const int indent = calcIndent( tabStopWidthBySpace(), text );
        if( indent > 0 )
        {
            const QStringList lines = formattedText.split( '\n' );
            formattedText.clear();
            for( int i = 0; i < lines.size(); ++i )
            {
                const QString& line = lines.at( i );
                if( !line.isEmpty() )
                {
                    formattedText += QString( indent * tabStopWidthBySpace(), ' ' );
                    formattedText += line;
                }
                if( i < lines.size() - 1 )
                {
                    formattedText += '\n';
                }
            }
        }

        textCursor.beginEditBlock();
        textCursor.deleteChar();
        textCursor.insertText( formattedText );
        textCursor.endEditBlock();
    }
}
