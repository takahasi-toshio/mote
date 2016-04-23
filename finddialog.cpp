#include "finddialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "settings.h"

namespace mote
{
    FindDialog::FindDialog( QWidget* parent )
        : QDialog( parent ),
          m_replaceMode( true )
    {
        m_lineEdit = new QLineEdit;
        m_replaceEdit = new QLineEdit;
        m_caseSensitive = new QCheckBox( tr( "Case sensitive" ) );
        m_wholeWords = new QCheckBox( tr( "Whole words" ) );
        m_regularExpression = new QCheckBox( tr( "Regular expression" ) );
        m_highlightAllOccurrences = new QCheckBox( tr( "Highlight all occurrences" ) );

        QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Close );
        buttonBox->addButton( tr( "Find" ), QDialogButtonBox::AcceptRole );

        connect( buttonBox, SIGNAL( accepted() ), SLOT( accept() ) );
        connect( buttonBox, SIGNAL( rejected() ), SLOT( reject() ) );

        m_formLayout = new QFormLayout;
        m_formLayout->setFieldGrowthPolicy( QFormLayout::AllNonFixedFieldsGrow );
        m_formLayout->setFormAlignment( Qt::AlignLeft | Qt::AlignTop );
        m_formLayout->addRow( tr( "Find:" ), m_lineEdit );
        m_formLayout->addRow( tr( "Replace:" ), m_replaceEdit );

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addLayout( m_formLayout );
        layout->addWidget( m_caseSensitive );
        layout->addWidget( m_wholeWords );
        layout->addWidget( m_regularExpression );
        layout->addWidget( m_highlightAllOccurrences );
        layout->addStretch();
        layout->addWidget( buttonBox );
        setLayout( layout );

        m_highlightAllOccurrences->hide(); // TODO

        setReplaceMode( false );
    }

    QSize FindDialog::sizeHint()const
    {
        return m_preferredSize.isValid() ? m_preferredSize : QDialog::sizeHint();
    }

    bool FindDialog::isReplaceMode( void )const
    {
        return m_replaceMode;
    }

    void FindDialog::setReplaceMode( const bool onoff )
    {
        if( onoff && !m_replaceMode )
        {
            setWindowTitle( tr( "Replace" ) );
            m_replaceEdit->show();
            m_formLayout->labelForField( m_replaceEdit )->show();
            m_replaceMode = true;
        }
        else if( !onoff && m_replaceMode )
        {
            setWindowTitle( tr( "Find" ) );
            m_replaceEdit->hide();
            m_formLayout->labelForField( m_replaceEdit )->hide();
            m_replaceMode = false;
        }
    }

    QString FindDialog::text( void )const
    {
        return m_lineEdit->text();
    }

    void FindDialog::setText( const QString& text )
    {
        m_lineEdit->setText( text );
        m_lineEdit->setFocus();
        m_lineEdit->selectAll();
    }

    QString FindDialog::after( void )const
    {
        return m_replaceEdit->text();
    }

    void FindDialog::setAfter( const QString& text )
    {
        m_replaceEdit->setText( text );
    }

    bool FindDialog::caseSensitivity( void )const
    {
        return m_caseSensitive->isChecked();
    }

    void FindDialog::setCaseSensitivity( const bool onoff )
    {
        m_caseSensitive->setChecked( onoff );
    }

    bool FindDialog::wholeWords( void )const
    {
        return m_wholeWords->isChecked();
    }

    void FindDialog::setWholeWords( const bool onoff )
    {
        m_wholeWords->setChecked( onoff );
    }

    bool FindDialog::isRegularExpressionEnabled( void )const
    {
        return m_regularExpression->isChecked();
    }

    void FindDialog::setRegularExpressionEnabled( const bool onoff )
    {
        m_regularExpression->setChecked( onoff );
    }

    bool FindDialog::highlightAllOccurrences( void )const
    {
        return m_highlightAllOccurrences->isChecked();
    }

    void FindDialog::setHighlightAllOccurrences( const bool onoff )
    {
        m_highlightAllOccurrences->setChecked( onoff );
    }

    void FindDialog::saveSize( Settings* settings )const
    {
        settings->setValue( "findDialogWidth", width() );
        settings->setValue( "findDialogHeight", height() );
    }

    void FindDialog::restoreSize( Settings* settings )
    {
        const QVariant width = settings->value( "findDialogWidth" );
        const QVariant height = settings->value( "findDialogHeight" );
        if( width.isValid() && height.isValid() )
        {
            m_preferredSize.setWidth( width.toInt() );
            m_preferredSize.setHeight( height.toInt() );
        }
    }
}
