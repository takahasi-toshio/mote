#pragma once

#include <QByteArray>
#include <QFile>
#include <QSyntaxHighlighter>
#include <QTextCodec>
#include <QTextDocument>

namespace mote
{
    class TextDocument : public QTextDocument
    {
        Q_OBJECT
        Q_PROPERTY( QString filePath READ filePath )
        Q_PROPERTY( QFont font READ font WRITE setFont )
        Q_PROPERTY( QString newlineCharacter READ newlineCharacter WRITE setNewlineCharacter )

    public:
        TextDocument( QObject* parent = 0 );

    public:
        QString filePath( void )const;
        QString fileName( void )const;

        QFont font( void )const;
        void setFont( const QFont& font );

        QString newlineCharacter( void )const;
        void setNewlineCharacter( const QString& newlineCharacter );

        QTextCodec* textCodec( void )const;
        void setTextCodec( QTextCodec* codec );

        bool generateByteOrderMark( void )const;
        void setGenerateByteOrderMark( const bool onoff );

    public:
        bool openFile( const QString& path );
        bool saveFile( const QString& path );
        bool reload( void );

    signals:
        void fontChanged( void );
        void filePathChanged( TextDocument* textDocument );
        void newlineCharacterChanged( TextDocument* textDocument );
        void textCodecChanged( TextDocument* textDocument );
        void generateByteOrderMarkChanged( TextDocument* textDocument );

    private:
        QTextCodec* detectCodec(
            const QByteArray& data,
            QByteArray& bom )const;
        QString detectNewlineCharacter( const QString& str )const;
        void writeBOM( QFile& file )const;

    private slots:
        void onFilePathChanged( void );

    private:
        QByteArray m_data;
        bool m_readOnly;
        bool m_generateBOM;
        QTextCodec* m_textCodec;
        QString m_newlineChar;
        QSyntaxHighlighter* m_syntaxHighlighter;
    };
}
