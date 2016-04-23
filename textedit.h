#pragma once

#include <QList>
#include <QListWidget>
#include <QPlainTextEdit>

namespace mote
{
    class TextDocument;

    class TextEdit : public QPlainTextEdit
    {
        Q_OBJECT

    public:
        TextEdit( TextDocument* document, QWidget* parent = 0 );

    public:
        virtual bool eventFilter( QObject* watched, QEvent* event );
        virtual QSize sizeHint()const;

    public:
        bool isLineNumberVisible( void )const;
        void setLineNumberVisible( const bool onoff );

        int tabStopWidthBySpace( void )const;
        void setTabStopWidthBySpace( const int count );

        static QList<TextEdit*> findEdits( TextDocument* textDocument = NULL );
        static QList<TextEdit*> findEdits( const QString& path );

    public slots:
        void formatSourceCode( void );
        void jumpToCoBrace( void );
        void findNext( const QString& text, const QTextDocument::FindFlags flags );
        void findNext( const QRegExp& expr, const QTextDocument::FindFlags flags );
        void findPrevious( const QString& text, const QTextDocument::FindFlags flags );
        void findPrevious( const QRegExp& expr, const QTextDocument::FindFlags flags );
        void replaceNext( const QString& text, const QString& after, const QTextDocument::FindFlags flags );
        void replaceNext( const QRegExp& expr, const QString& after, const QTextDocument::FindFlags flags );
        void replacePrevious( const QString& text, const QString& after, const QTextDocument::FindFlags flags );
        void replacePrevious( const QRegExp& expr, const QString& after, const QTextDocument::FindFlags flags );

    protected:
        virtual void resizeEvent( QResizeEvent* event );
        virtual void paintEvent( QPaintEvent* event );
        virtual void keyPressEvent( QKeyEvent* event );
        virtual void dragEnterEvent( QDragEnterEvent* event );
        virtual void dragMoveEvent( QDragMoveEvent* event );
        virtual void dropEvent( QDropEvent* event );

    private:
        void updateViewportMargins( const bool force = false );
        void updateLineNumberWidgetGeometry( void );
        void updateTabStopWidthBySpace( void );
        void updateExtraSelections();
        void updateCoBracePos();
        void drawLineNumber( void );
        void drawEOF( void );
        void drawNewlineCharacter( void );
        void drawTabCharacter( void );
        void drawGuideLine();
        void indent( void );
        void reverseIndent( void );
        bool isPartOfString( const int pos )const;
        void inputCompletion( void );
        bool isInputCompletionVisible( void )const;
        void applyInputCompletion( void );
        void autoIndent( void );
        QTextCursor _findNext( const QString& text, const QTextDocument::FindFlags flags )const;
        QTextCursor _findNext( const QRegExp& expr, const QTextDocument::FindFlags flags )const;
        QTextCursor _findPrevious( const QString& text, const QTextDocument::FindFlags flags )const;
        QTextCursor _findPrevious( const QRegExp& expr, const QTextDocument::FindFlags flags )const;

    private slots:
        void onFontChanged( void );
        void onCursorPositionChanged();
        void onSelectionChanged();
        void onTextChanged();

    private:
        bool m_lineNumberVisible;
        int m_lineNumberWidth;
        int m_lineNumberPixelWidth;
        QWidget* m_lineNumberWidget;
        int m_tabStopWidthBySpace;
        int m_rowSelectionBasePos;
        int m_coBracePos[2];
        QListWidget* m_inputCompletionList;
    };
}
