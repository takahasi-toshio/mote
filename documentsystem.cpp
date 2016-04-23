#include "documentsystem.h"

#include "settings.h"
#include "textdocument.h"

namespace mote
{
    DocumentSystem::DocumentSystem( Settings* settings, QObject* parent )
        : QObject( parent )
    {
        connect(
            settings, SIGNAL( fontChanged( const QFont& ) ),
            SLOT( onFontChanged( const QFont& ) ) );
    }

    TextDocument* DocumentSystem::createDocument( void )
    {
        TextDocument* textDocument = new TextDocument( this );
        connect(
            textDocument, SIGNAL( filePathChanged( TextDocument* ) ),
            SIGNAL( filePathChanged( TextDocument* ) ) );
        connect(
            textDocument, SIGNAL( modificationChanged( bool ) ),
            SLOT( onModificationChanged( void ) ) );
        connect(
            textDocument, SIGNAL( newlineCharacterChanged( TextDocument* ) ),
            SIGNAL( newlineCharacterChanged( TextDocument* ) ) );
        connect(
            textDocument, SIGNAL( textCodecChanged( TextDocument* ) ),
            SIGNAL( textCodecChanged( TextDocument* ) ) );
        connect(
            textDocument, SIGNAL( generateByteOrderMarkChanged( TextDocument* ) ),
            SIGNAL( generateByteOrderMarkChanged( TextDocument* ) ) );
        return textDocument;
    }

    void DocumentSystem::onModificationChanged( void )
    {
        emit modificationChanged( qobject_cast<TextDocument*>( sender() ) );
    }

    void DocumentSystem::onFontChanged( const QFont& font )
    {
        const QList<TextDocument*> textDocuments = findChildren<TextDocument*>();
        for( int i = 0; i < textDocuments.size(); ++i )
        {
            TextDocument* textDocument = textDocuments.at( i );
            if( textDocument )
            {
                textDocument->setFont( font );
            }
        }
    }
}
