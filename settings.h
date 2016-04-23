#pragma once

#include <QFont>
#include <QSettings>

namespace mote
{
    class Settings : public QSettings
    {
        Q_OBJECT

    public:
        Settings( void );

    public:
        bool isLineNumberVisible( void )const;
        QFont font( void )const;
        bool findCaseSensitivity( void )const;
        bool findWholeWords( void )const;
        bool isFindRegularExpressionEnabled( void )const;
        bool findHighlightAllOccurrences( void )const;

    public slots:
        void setLineNumberVisible( bool onoff );
        void setFont( const QFont& font );
        void setFindCaseSensitivity( const bool onoff );
        void setFindWholeWords( const bool onoff );
        void setFindRegularExpressionEnabled( const bool onoff );
        void setFindHighlightAllOccurrences( const bool onoff );

    signals:
        void lineNumberVisibilityChanged( bool onoff );
        void fontChanged( const QFont& font );
    };
}
