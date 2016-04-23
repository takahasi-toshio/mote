#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>

namespace mote
{
    class Settings;

    class FindDialog : public QDialog
    {
        Q_OBJECT

    public:
        FindDialog( QWidget* parent = 0 );

    public:
        virtual QSize sizeHint()const;

    public:
        bool isReplaceMode( void )const;
        void setReplaceMode( const bool onoff );

        QString text( void )const;
        void setText( const QString& text );

        QString after( void )const;
        void setAfter( const QString& text );

        bool caseSensitivity( void )const;
        void setCaseSensitivity( const bool onoff );

        bool wholeWords( void )const;
        void setWholeWords( const bool onoff );

        bool isRegularExpressionEnabled( void )const;
        void setRegularExpressionEnabled( const bool onoff );

        bool highlightAllOccurrences( void )const;
        void setHighlightAllOccurrences( const bool onoff );

        void saveSize( Settings* settings )const;
        void restoreSize( Settings* settings );

    private:
        bool m_replaceMode;
        QFormLayout* m_formLayout;
        QLineEdit* m_lineEdit;
        QLineEdit* m_replaceEdit;
        QCheckBox* m_caseSensitive;
        QCheckBox* m_wholeWords;
        QCheckBox* m_regularExpression;
        QCheckBox* m_highlightAllOccurrences;
        QSize m_preferredSize;
    };
}
