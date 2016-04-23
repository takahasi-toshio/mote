#include "cppsyntaxhighlighter.h"

#define STATE_VALUE_MASK          0x00000FFF
#define STATE_FLAGS_MASK          0x7FFFF000

#define STATE_DOUBLE_QUOTE_STRING 0x00000001
#define STATE_SINGLE_QUOTE_STRING 0x00000002
#define STATE_LINE_COMMENT        0x00000003
#define STATE_BLOCK_COMMENT       0x00000004


namespace mote
{
    CppSyntaxHighlighter::CppSyntaxHighlighter( QTextDocument* parent )
        : QSyntaxHighlighter( parent )
    {
        m_stringFormat.setForeground( Qt::red );
        m_stringFormat.setProperty( QTextFormat::UserProperty + 1, true );

        m_commentFormat.setForeground( Qt::darkGreen );
        m_commentFormat.setBackground( QColor( 216, 216, 216 ) );
    }

    void CppSyntaxHighlighter::highlightBlock( const QString& text )
    {
        int state = previousBlockState();
        if( state == -1 )
        {
            state = 0;
        }

        int stateValue = ( state & STATE_VALUE_MASK );
        int stateFlags = ( state & STATE_FLAGS_MASK );

        bool escapeString = false;
        int startPos = 0;
        for( int i = 0; i < text.length(); ++i )
        {
            if( escapeString )
            {
                escapeString = false;
                continue;
            }

            const QChar ch = text[i];

            if( ch == '\\' )
            {
                if( ( stateValue == STATE_DOUBLE_QUOTE_STRING ) ||
                    ( stateValue == STATE_SINGLE_QUOTE_STRING ) )
                {
                    escapeString = true;
                    continue;
                }
            }

            if( ch == '"' )
            {
                if( stateValue == STATE_DOUBLE_QUOTE_STRING )
                {
                    setFormat( startPos, i - startPos + 1, m_stringFormat );
                    stateValue = 0;
                }
                else if( stateValue == 0 )
                {
                    stateValue = STATE_DOUBLE_QUOTE_STRING;
                    startPos = i;
                }
            }
            else if( ch == '\'' )
            {
                if( stateValue == STATE_SINGLE_QUOTE_STRING )
                {
                    setFormat( startPos, i - startPos + 1, m_stringFormat );
                    stateValue = 0;
                }
                else if( stateValue == 0 )
                {
                    stateValue = STATE_SINGLE_QUOTE_STRING;
                    startPos = i;
                }
            }
            else if( ch == '/' )
            {
                if( stateValue == STATE_BLOCK_COMMENT )
                {
                    if( i > 0 )
                    {
                        const QChar prevCh = text[i - 1];
                        if( prevCh == '*' )
                        {
                            setFormat( startPos, i - startPos + 1, m_commentFormat );
                            stateValue = 0;
                        }
                    }
                }
                else if( stateValue == 0 )
                {
                    if( i + 1 < text.length() )
                    {
                        const QChar nextCh = text[i + 1];
                        if( nextCh == '/' )
                        {
                            stateValue = STATE_LINE_COMMENT;
                            startPos = i;
                        }
                        else if( nextCh == '*' )
                        {
                            stateValue = STATE_BLOCK_COMMENT;
                            startPos = i;
                        }
                    }
                }
            }
        }

        switch( stateValue )
        {
        case STATE_DOUBLE_QUOTE_STRING:
        case STATE_SINGLE_QUOTE_STRING:
            setFormat( startPos, text.length() - startPos, m_stringFormat );
            if( !escapeString )
            {
                stateValue = 0;
            }
            else
            {
                // continuation line
            }
            break;
        case STATE_LINE_COMMENT:
            setFormat( startPos, text.length() - startPos, m_commentFormat );
            if( !text.endsWith( "\\" ) )
            {
                stateValue = 0;
            }
            else
            {
                // continuation line
            }
            break;
        case STATE_BLOCK_COMMENT:
            setFormat( startPos, text.length() - startPos, m_commentFormat );
            break;
        default:
            break;
        }

        if( stateValue | stateFlags )
        {
            setCurrentBlockState( stateValue | stateFlags );
        }
        else
        {
            setCurrentBlockState( -1 );
        }
    }
}
