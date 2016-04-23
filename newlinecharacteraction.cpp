#include "newlinecharacteraction.h"

#include <QComboBox>

#include "documentsystem.h"
#include "mainwindow.h"

namespace mote
{
    NewlineCharacterAction::NewlineCharacterAction(
        DocumentSystem* documentSystem,
        MainWindow* mainWindow )
        : QWidgetAction( mainWindow )
    {
        m_document = mainWindow->currentDocument();

        connect(
            documentSystem, SIGNAL( newlineCharacterChanged( TextDocument* ) ),
            SLOT( onDocumentNewlineCharacterChanged( TextDocument* ) ) );

        connect(
            mainWindow, SIGNAL( currentDocumentChanged( TextDocument* ) ),
            SLOT( setDocument( TextDocument* ) ) );
    }

    QWidget* NewlineCharacterAction::createWidget( QWidget* parent )
    {
        QComboBox* comboBox = new QComboBox( parent );
        comboBox->setFocusPolicy( Qt::NoFocus );
        comboBox->addItem( tr( "CR" ) );
        comboBox->addItem( tr( "LF" ) );
        comboBox->addItem( tr( "CRLF" ) );

        if( m_document )
        {
            updateValue( comboBox, m_document->newlineCharacter() );
        }

        connect(
            comboBox, SIGNAL( currentIndexChanged( int ) ),
            SLOT( onNewlineCharacterChanged( int ) ) );

        return comboBox;
    }

    void NewlineCharacterAction::updateValue( QWidget* w, const QString& newlineCharacter )const
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>( w );
        if( comboBox )
        {
            comboBox->blockSignals( true );
            if( newlineCharacter == "\r" )
            {
                comboBox->setCurrentIndex( 0 );
            }
            else if( newlineCharacter == "\n" )
            {
                comboBox->setCurrentIndex( 1 );
            }
            else
            {
                comboBox->setCurrentIndex( 2 );
            }
            comboBox->blockSignals( false );
        }
    }

    void NewlineCharacterAction::onDocumentNewlineCharacterChanged( TextDocument* textDocument )
    {
        if( textDocument != m_document )
        {
            return;
        }

        if( m_document )
        {
            const QList<QWidget*> widgets = createdWidgets();
            for( int i = 0; i < widgets.size(); ++i )
            {
                updateValue( widgets.at( i ), m_document->newlineCharacter() );
            }
        }
    }

    void NewlineCharacterAction::onNewlineCharacterChanged( int index )
    {
        if( m_document )
        {
            switch( index )
            {
            case 0:
                m_document->setNewlineCharacter( "\r" );
                break;
            case 1:
                m_document->setNewlineCharacter( "\n" );
                break;
            default:
                m_document->setNewlineCharacter( "\r\n" );
                break;
            }
        }
    }

    void NewlineCharacterAction::setDocument( TextDocument* textDocument )
    {
        m_document = textDocument;
        onDocumentNewlineCharacterChanged( textDocument );

    }
}
