#pragma once

#include <QFont>
#include <QObject>

namespace mote
{
    class Settings;
    class TextDocument;

    class DocumentSystem : public QObject
    {
        Q_OBJECT

    public:
        DocumentSystem( Settings* settings, QObject* parent = 0 );

    public:
        TextDocument* createDocument( void );

    signals:
        void filePathChanged( TextDocument* textDocument );
        void modificationChanged( TextDocument* textDocument );
        void newlineCharacterChanged( TextDocument* textDocument );
        void textCodecChanged( TextDocument* textDocument );
        void generateByteOrderMarkChanged( TextDocument* textDocument );

    private slots:
        void onModificationChanged( void );
        void onFontChanged( const QFont& font );
    };
}
