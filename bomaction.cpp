#include "bomaction.h"

#include <QCheckBox>
#include <QTextCodec>

#include "documentsystem.h"
#include "mainwindow.h"

namespace mote
{
    BOMAction::BOMAction(
        DocumentSystem* documentSystem,
        MainWindow* mainWindow )
        : QWidgetAction( mainWindow )
    {
        m_document = mainWindow->currentDocument();

        connect(
            documentSystem, SIGNAL( textCodecChanged( TextDocument* ) ),
            SLOT( onDocumentValueChanged( TextDocument* ) ) );
        connect(
            documentSystem, SIGNAL( generateByteOrderMarkChanged( TextDocument* ) ),
            SLOT( onDocumentValueChanged( TextDocument* ) ) );

        connect(
            mainWindow, SIGNAL( currentDocumentChanged( TextDocument* ) ),
            SLOT( setDocument( TextDocument* ) ) );
    }

    QWidget* BOMAction::createWidget( QWidget* parent )
    {
        QCheckBox* checkBox = new QCheckBox( tr( "BOM" ), parent );
        connect(
            checkBox, SIGNAL( clicked( bool ) ),
            SLOT( onBOMChanged( bool ) ) );
        checkBox->setFocusPolicy( Qt::NoFocus );

        if( m_document )
        {
            updateValue(
                checkBox,
                m_document->textCodec(),
                m_document->generateByteOrderMark() );
        }

        return checkBox;
    }

    void BOMAction::updateValue(
        QWidget* w,
        QTextCodec* codec,
        const bool generateByteOrderMark )const
    {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>( w );
        if( checkBox )
        {
            checkBox->blockSignals( true );
            const QString name = codec->name().constData();
            const bool enabled = ( ( name == "UTF-8" ) ||
                                   ( name == "UTF-16" ) ||
                                   ( name == "UTF-16LE" ) ||
                                   ( name == "UTF-16BE" ) ||
                                   ( name == "UTF-32" ) ||
                                   ( name == "UTF-32LE" ) ||
                                   ( name == "UTF-32BE" ) );
            checkBox->setEnabled( enabled );
            checkBox->setChecked( enabled && generateByteOrderMark );
            checkBox->blockSignals( false );
        }
    }

    void BOMAction::onDocumentValueChanged( TextDocument* textDocument )
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
                updateValue(
                    widgets.at( i ),
                    m_document->textCodec(),
                    m_document->generateByteOrderMark() );
            }
        }
    }

    void BOMAction::onBOMChanged( bool onoff )
    {
        if( m_document )
        {
            m_document->setGenerateByteOrderMark( onoff );
        }
    }

    void BOMAction::setDocument( TextDocument* textDocument )
    {
        m_document = textDocument;
        onDocumentValueChanged( textDocument );
    }
}
