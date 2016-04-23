#pragma once

#include <QSyntaxHighlighter>

namespace mote
{
    class CppSyntaxHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT

    public:
        CppSyntaxHighlighter( QTextDocument* parent );

    protected:
        virtual void highlightBlock( const QString& text );

    private:
        QTextCharFormat m_stringFormat;
        QTextCharFormat m_commentFormat;
    };
}
