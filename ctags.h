#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QTextDocument>

namespace mote
{
    class CTags
    {
    public:
        CTags( void );

    public:
        bool exec( const QTextDocument* document );

    public:
        struct Entry
        {
            QString tagName;
            QString fileName;
            QString exCmd;
            QStringList extensionFields;
            QString scope( void )const;
            QString kind( void )const;
        };

        int count( void )const;
        const Entry& entry( int index )const;

    private:
        QString findCTags( void )const;
        bool readTags( const QString& path );

    private:
        QList<Entry> m_entries;
    };
}
