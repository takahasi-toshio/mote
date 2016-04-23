#include "settings.h"

#include <QApplication>

namespace mote
{
    Settings::Settings( void )
        : QSettings( QSettings::UserScope, "motools", "mote" )
    {
    }

    bool Settings::isLineNumberVisible( void )const
    {
        const QVariant lineNumberVisible = value( "lineNumberVisible" );
        if( lineNumberVisible.isValid() )
        {
            return lineNumberVisible.toBool();
        }
        else
        {
            return false;
        }
    }

    QFont Settings::font( void )const
    {
        const QVariant fontFamily = value( "fontFamily" );
        const QVariant fontPointSize = value( "fontPointSize" );
        if( fontFamily.isValid() && fontPointSize.isValid() )
        {
            return QFont( fontFamily.toString(), fontPointSize.toInt() );
        }
        else
        {
            return QApplication::font( "QPlainTextEdit" );
        }
    }

    bool Settings::findCaseSensitivity( void )const
    {
        const QVariant findCaseSensitivity = value( "findCaseSensitivity" );
        if( findCaseSensitivity.isValid() )
        {
            return findCaseSensitivity.toBool();
        }
        else
        {
            return false;
        }
    }

    bool Settings::findWholeWords( void )const
    {
        const QVariant findWholeWords = value( "findWholeWords" );
        if( findWholeWords.isValid() )
        {
            return findWholeWords.toBool();
        }
        else
        {
            return false;
        }
    }

    bool Settings::isFindRegularExpressionEnabled( void )const
    {
        const QVariant findRegularExpressionEnabled =
            value( "findRegularExpressionEnabled" );
        if( findRegularExpressionEnabled.isValid() )
        {
            return findRegularExpressionEnabled.toBool();
        }
        else
        {
            return false;
        }
    }

    bool Settings::findHighlightAllOccurrences( void )const
    {
        const QVariant findHighlightAllOccurrences =
            value( "findHighlightAllOccurrences" );
        if( findHighlightAllOccurrences.isValid() )
        {
            return findHighlightAllOccurrences.toBool();
        }
        else
        {
            return false;
        }
    }

    void Settings::setLineNumberVisible( bool onoff )
    {
        const bool prevOnoff = isLineNumberVisible();
        if( ( prevOnoff && !onoff ) || ( !prevOnoff && onoff ) )
        {
            setValue( "lineNumberVisible", onoff );
            emit lineNumberVisibilityChanged( onoff );
        }
    }

    void Settings::setFont( const QFont& font )
    {
        setValue( "fontFamily", font.family() );
        setValue( "fontPointSize", font.pointSize() );
        emit fontChanged( this->font() );
    }

    void Settings::setFindCaseSensitivity( const bool onoff )
    {
        setValue( "findCaseSensitivity", onoff );
    }

    void Settings::setFindWholeWords( const bool onoff )
    {
        setValue( "findWholeWords", onoff );
    }

    void Settings::setFindRegularExpressionEnabled( const bool onoff )
    {
        setValue( "findRegularExpressionEnabled", onoff );
    }

    void Settings::setFindHighlightAllOccurrences( const bool onoff )
    {
        setValue( "findHighlightAllOccurrences", onoff );
    }
}
