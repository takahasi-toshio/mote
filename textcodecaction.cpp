#include "textcodecaction.h"

#include <QComboBox>
#include <QTextCodec>

#include "documentsystem.h"
#include "mainwindow.h"

namespace mote
{
    TextCodecAction::TextCodecAction(
        DocumentSystem* documentSystem,
        MainWindow* mainWindow )
        : QWidgetAction( mainWindow )
    {
        m_document = mainWindow->currentDocument();

        connect(
            documentSystem, SIGNAL( textCodecChanged( TextDocument* ) ),
            SLOT( onDocumentTextCodecChanged( TextDocument* ) ) );

        connect(
            mainWindow, SIGNAL( currentDocumentChanged( TextDocument* ) ),
            SLOT( setDocument( TextDocument* ) ) );
    }

    QWidget* TextCodecAction::createWidget( QWidget* parent )
    {
        QComboBox* comboBox = new QComboBox( parent );
        connect(
            comboBox, SIGNAL( currentIndexChanged( const QString& ) ),
            SLOT( onTextCodecChanged( const QString& ) ) );
        comboBox->setFocusPolicy( Qt::NoFocus );

        comboBox->addItem( QTextCodec::codecForLocale()->name().constData() );

        const QList<QByteArray> codecs = QTextCodec::availableCodecs();
        for( int i = 0; i < codecs.size(); ++i )
        {
            QTextCodec* codec = QTextCodec::codecForName( codecs.at( i ) );
            if( codec )
            {
                if( comboBox->findText( codec->name().constData() ) == -1 )
                {
                    comboBox->addItem( codec->name().constData() );
                }
            }
        }

        if( m_document )
        {
            updateValue( comboBox, m_document->textCodec() );
        }

        return comboBox;
    }

    void TextCodecAction::updateValue( QWidget* w, QTextCodec* codec )const
    {
        QComboBox* comboBox = qobject_cast<QComboBox*>( w );
        if( comboBox )
        {
            comboBox->blockSignals( true );
            comboBox->setCurrentIndex(
                comboBox->findText( codec->name().constData() ) );
            comboBox->blockSignals( false );
        }
    }

    void TextCodecAction::onDocumentTextCodecChanged( TextDocument* textDocument )
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
                updateValue( widgets.at( i ), m_document->textCodec() );
            }
        }
    }

    void TextCodecAction::onTextCodecChanged( const QString& text )
    {
        if( m_document )
        {
            QTextCodec* codec = QTextCodec::codecForName( text.toLocal8Bit() );
            if( codec )
            {
                m_document->setTextCodec( codec );
            }
        }
    }

    void TextCodecAction::setDocument( TextDocument* textDocument )
    {
        m_document = textDocument;
        onDocumentTextCodecChanged( textDocument );

    }
}
