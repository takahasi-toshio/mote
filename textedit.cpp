#include "textedit.h"

#include <QApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFontMetrics>
#include <QMap>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QRegExp>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QUrl>
#include <QVector>

#include "inputcompletionitemdelegate.h"
#include "mainwindow.h"
#include "textdocument.h"

namespace mote
{
    TextEdit::TextEdit( TextDocument* document, QWidget* parent )
        : QPlainTextEdit( parent ),
          m_lineNumberVisible( false ),
          m_lineNumberWidth( 0 ),
          m_lineNumberPixelWidth( 0 ),
          m_lineNumberWidget( NULL ),
          m_tabStopWidthBySpace( 4 ),
          m_rowSelectionBasePos( 0 ),
          m_inputCompletionList( NULL )
    {
        m_coBracePos[0] = -1;
        m_coBracePos[1] = -1;

        setDocument( document );
        connect(
            document,
            SIGNAL( fontChanged( void ) ), SLOT( onFontChanged( void ) ) );

        setLineWrapMode( QPlainTextEdit::NoWrap );
        setCursorWidth( 2 );

        updateTabStopWidthBySpace();
        updateExtraSelections();

        connect(
            this, SIGNAL( cursorPositionChanged() ),
            SLOT( onCursorPositionChanged() ) );
        connect(
            this, SIGNAL( selectionChanged() ),
            SLOT( onSelectionChanged() ) );
        connect(
            this, SIGNAL( textChanged() ),
            SLOT( onTextChanged() ) );
    }

    bool TextEdit::eventFilter( QObject* watched, QEvent* event )
    {
        if( watched == m_lineNumberWidget )
        {
            if( event->type() == QEvent::Paint )
            {
                drawLineNumber();
                return true;
            }
            else if( event->type() == QEvent::MouseButtonPress )
            {
                QMouseEvent* mouseEvent = ( QMouseEvent* )event;
                if( mouseEvent->button() == Qt::LeftButton )
                {
                    QTextCursor textCursor =
                        cursorForPosition( QPoint( 0, mouseEvent->pos().y() ) );
                    if( !textCursor.isNull() )
                    {
                        textCursor.movePosition( QTextCursor::StartOfBlock );
                        m_rowSelectionBasePos = textCursor.position();
                        if( textCursor.block().next().isValid() )
                        {
                            textCursor.movePosition( QTextCursor::NextBlock );
                        }
                        else
                        {
                            textCursor.movePosition( QTextCursor::EndOfBlock );
                        }
                        textCursor.setPosition( m_rowSelectionBasePos, QTextCursor::KeepAnchor );
                        setTextCursor( textCursor );
                    }
                }
                return true;
            }
            else if( event->type() == QEvent::MouseMove )
            {
                QMouseEvent* mouseEvent = ( QMouseEvent* )event;
                if( mouseEvent->buttons() & Qt::LeftButton )
                {
                    QTextCursor textCursor =
                        cursorForPosition( QPoint( 0, mouseEvent->pos().y() ) );
                    if( !textCursor.isNull() )
                    {
                        textCursor.movePosition( QTextCursor::StartOfBlock );
                        const int pos = textCursor.position();
                        if( pos == m_rowSelectionBasePos )
                        {
                            if( textCursor.block().next().isValid() )
                            {
                                textCursor.movePosition( QTextCursor::NextBlock );
                            }
                            else
                            {
                                textCursor.movePosition( QTextCursor::EndOfBlock );
                            }
                            textCursor.setPosition( pos, QTextCursor::KeepAnchor );
                            setTextCursor( textCursor );
                        }
                        else if( pos < m_rowSelectionBasePos )
                        {
                            textCursor.setPosition( m_rowSelectionBasePos );
                            if( textCursor.block().next().isValid() )
                            {
                                textCursor.movePosition( QTextCursor::NextBlock );
                            }
                            else
                            {
                                textCursor.movePosition( QTextCursor::EndOfBlock );
                            }
                            textCursor.setPosition( pos, QTextCursor::KeepAnchor );
                            setTextCursor( textCursor );
                        }
                        else
                        {
                            textCursor.setPosition( m_rowSelectionBasePos );
                            textCursor.setPosition( pos, QTextCursor::KeepAnchor );
                            if( textCursor.block().next().isValid() )
                            {
                                textCursor.movePosition(
                                    QTextCursor::NextBlock,
                                    QTextCursor::KeepAnchor );
                            }
                            else
                            {
                                textCursor.movePosition(
                                    QTextCursor::EndOfBlock,
                                    QTextCursor::KeepAnchor );
                            }
                            setTextCursor( textCursor );
                        }
                    }
                }
                return true;
            }
        }

        return QPlainTextEdit::eventFilter( watched, event );
    }

    QSize TextEdit::sizeHint()const
    {
        QFontMetrics fontMetrics( document()->defaultFont() );
        return QSize( fontMetrics.width( "W" ) * 80, fontMetrics.height() * 25 );
    }

    bool TextEdit::isLineNumberVisible( void )const
    {
        return m_lineNumberVisible;
    }

    void TextEdit::setLineNumberVisible( const bool onoff )
    {
        if( onoff )
        {
            if( !m_lineNumberVisible )
            {
                m_lineNumberVisible = true;
                m_lineNumberWidth = 0;
                if( !m_lineNumberWidget )
                {
                    m_lineNumberWidget = new QWidget( this );
                    m_lineNumberWidget->installEventFilter( this );
                    m_lineNumberWidget->connect(
                        this, SIGNAL( updateRequest( const QRect&, int ) ),
                        SLOT( update() ) );
                }
                updateViewportMargins();
                updateLineNumberWidgetGeometry();
                m_lineNumberWidget->show();
            }
        }
        else
        {
            if( m_lineNumberVisible )
            {
                m_lineNumberVisible = false;
                m_lineNumberWidth = 0;
                if( m_lineNumberWidget )
                {
                    m_lineNumberWidget->hide();
                }
                updateViewportMargins();
            }
        }
    }

    int TextEdit::tabStopWidthBySpace( void )const
    {
        return m_tabStopWidthBySpace;
    }

    void TextEdit::setTabStopWidthBySpace( const int count )
    {
        if( count != m_tabStopWidthBySpace )
        {
            m_tabStopWidthBySpace = count;
            updateTabStopWidthBySpace();
        }
    }

    QList<TextEdit*> TextEdit::findEdits( TextDocument* textDocument )
    {
        QList<TextEdit*> edits;
        const QWidgetList wins = QApplication::topLevelWidgets();
        for( int i = 0; i < wins.size(); ++i )
        {
            const MainWindow* win = qobject_cast<MainWindow*>( wins.at( i ) );
            if( win )
            {
                edits.append( win->findEdits( textDocument ) );
            }
        }
        return edits;
    }

    QList<TextEdit*> TextEdit::findEdits( const QString& path )
    {
        QList<TextEdit*> edits;
        const QWidgetList wins = QApplication::topLevelWidgets();
        for( int i = 0; i < wins.size(); ++i )
        {
            const MainWindow* win = qobject_cast<MainWindow*>( wins.at( i ) );
            if( win )
            {
                edits.append( win->findEdits( path ) );
            }
        }
        return edits;
    }

    void TextEdit::resizeEvent( QResizeEvent* event )
    {
        if( m_lineNumberWidget )
        {
            updateLineNumberWidgetGeometry();
        }

        QPlainTextEdit::resizeEvent( event );
    }

    void TextEdit::jumpToCoBrace( void )
    {
        if( m_coBracePos[1] >= 0 )
        {
            QTextCursor textCursor = this->textCursor();
            textCursor.setPosition( m_coBracePos[1] );
            setTextCursor( textCursor );
        }
    }

    void TextEdit::findNext( const QString& text, const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findNext( text, flags );
        if( !textCursor.isNull() )
        {
            setTextCursor( textCursor );
        }
    }

    void TextEdit::findNext( const QRegExp& expr, const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findNext( expr, flags );
        if( !textCursor.isNull() )
        {
            setTextCursor( textCursor );
        }
    }

    void TextEdit::findPrevious( const QString& text, const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findPrevious( text, flags );
        if( !textCursor.isNull() )
        {
            setTextCursor( textCursor );
        }
    }

    void TextEdit::findPrevious( const QRegExp& expr, const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findPrevious( expr, flags );
        if( !textCursor.isNull() )
        {
            setTextCursor( textCursor );
        }
    }

    void TextEdit::replaceNext(
        const QString& text,
        const QString& after,
        const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findNext( text, flags );
        if( !textCursor.isNull() )
        {
            textCursor.beginEditBlock();
            textCursor.deleteChar();
            textCursor.insertText( after );
            textCursor.endEditBlock();
            setTextCursor( textCursor );
        }
    }

    void TextEdit::replaceNext(
        const QRegExp& expr,
        const QString& after,
        const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findNext( expr, flags );
        if( !textCursor.isNull() )
        {
            textCursor.beginEditBlock();
            textCursor.deleteChar();
            textCursor.insertText( after );
            textCursor.endEditBlock();
            setTextCursor( textCursor );
        }
    }

    void TextEdit::replacePrevious(
        const QString& text,
        const QString& after,
        const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findPrevious( text, flags );
        if( !textCursor.isNull() )
        {
            textCursor.beginEditBlock();
            textCursor.deleteChar();
            const int pos = textCursor.position();
            textCursor.insertText( after );
            textCursor.setPosition( pos );
            textCursor.endEditBlock();
            setTextCursor( textCursor );
        }
    }

    void TextEdit::replacePrevious(
        const QRegExp& expr,
        const QString& after,
        const QTextDocument::FindFlags flags )
    {
        QTextCursor textCursor = _findPrevious( expr, flags );
        if( !textCursor.isNull() )
        {
            textCursor.beginEditBlock();
            textCursor.deleteChar();
            const int pos = textCursor.position();
            textCursor.insertText( after );
            textCursor.setPosition( pos );
            textCursor.endEditBlock();
            setTextCursor( textCursor );
        }
    }

    void TextEdit::paintEvent( QPaintEvent* event )
    {
        QPlainTextEdit::paintEvent( event );

        drawEOF();
        drawNewlineCharacter();
        drawTabCharacter();
        drawGuideLine();
    }

    void TextEdit::keyPressEvent( QKeyEvent* event )
    {
        if( event->key() == Qt::Key_Escape )
        {
            QTextCursor cursor = textCursor();
            if( cursor.hasSelection() )
            {
                cursor.setPosition( cursor.anchor() );
                setTextCursor( cursor );
            }
            else
            {
                if( isInputCompletionVisible() )
                {
                    m_inputCompletionList->hide();
                }
                else
                {
                    inputCompletion();
                }
            }

            event->accept();
        }
        else if( ( event->key() == Qt::Key_C ) &&
                 ( event->modifiers() == Qt::ControlModifier ) )
        {
            copy();

            QTextCursor textCursor = this->textCursor();
            if( textCursor.hasSelection() )
            {
                textCursor.clearSelection();
                setTextCursor( textCursor );
            }

            event->accept();
        }
        else if( ( event->key() == Qt::Key_Backspace ) &&
                 ( event->modifiers() == Qt::ShiftModifier ) )
        {
            QKeyEvent deleteKeyEvent(
                event->type(),
                Qt::Key_Delete,
                Qt::NoModifier );
            QPlainTextEdit::keyPressEvent( &deleteKeyEvent );
        }
        else if( ( event->key() == Qt::Key_Backspace ) &&
                 ( event->modifiers() == Qt::ControlModifier ) )
        {
            reverseIndent();

            event->accept();
        }
        else if( ( event->key() == Qt::Key_Enter ) ||
                 ( event->key() == Qt::Key_Return ) )
        {
            if( isInputCompletionVisible() )
            {
                applyInputCompletion();

                m_inputCompletionList->hide();

                event->accept();
            }
            else
            {
                const int pos = textCursor().position();

                if( event->modifiers() == Qt::ShiftModifier )
                {
                    QKeyEvent noModEvent(
                        event->type(),
                        event->key(),
                        Qt::NoModifier );
                    QPlainTextEdit::keyPressEvent( &noModEvent );
                }
                else
                {
                    QPlainTextEdit::keyPressEvent( event );
                }

                if( textCursor().position() != pos )
                {
                    autoIndent();
                }
            }
        }
        else if( event->key() == Qt::Key_Tab )
        {
            indent();

            event->accept();
        }
        else if( ( event->text() == ")" ) ||
                 ( event->text() == "}" ) ||
                 ( event->text() == "]" ) ||
                 ( event->text() == ">" ) )
        {
            QPlainTextEdit::keyPressEvent( event );
            m_coBracePos[0] = textCursor().position() - 1;
            updateCoBracePos();
            updateExtraSelections();
        }
        else if( ( event->key() == Qt::Key_Up ) &&
                 isInputCompletionVisible() )
        {
            if( m_inputCompletionList->currentRow() > 0 )
            {
                m_inputCompletionList->setCurrentRow( m_inputCompletionList->currentRow() - 1 );
            }

            event->accept();
        }
        else if( ( event->key() == Qt::Key_Down ) &&
                 isInputCompletionVisible() )
        {
            if( m_inputCompletionList->currentRow() + 1 < m_inputCompletionList->count() )
            {
                m_inputCompletionList->setCurrentRow( m_inputCompletionList->currentRow() + 1 );
            }

            event->accept();
        }
        else if( ( event->key() == Qt::Key_Home ) && isInputCompletionVisible() )
        {
            m_inputCompletionList->setCurrentRow( 0 );

            event->accept();
        }
        else if( ( event->key() == Qt::Key_End ) && isInputCompletionVisible() )
        {
            m_inputCompletionList->setCurrentRow( m_inputCompletionList->count() - 1 );

            event->accept();
        }
        else
        {
            QPlainTextEdit::keyPressEvent( event );
        }
    }

    void TextEdit::dragEnterEvent( QDragEnterEvent* event )
    {
        if( event->mimeData()->hasUrls() )
        {
            const QList<QUrl> urls = event->mimeData()->urls();
            for( int i = 0; i < urls.size(); ++i )
            {
                const QUrl& url = urls.at( i );
                if( url.isLocalFile() )
                {
                    event->acceptProposedAction();
                    return;
                }
            }
        }

        QPlainTextEdit::dragEnterEvent( event );
    }

    void TextEdit::dragMoveEvent( QDragMoveEvent* event )
    {
        if( event->mimeData()->hasUrls() )
        {
            const QList<QUrl> urls = event->mimeData()->urls();
            for( int i = 0; i < urls.size(); ++i )
            {
                const QUrl& url = urls.at( i );
                if( url.isLocalFile() )
                {
                    event->acceptProposedAction();
                    return;
                }
            }
        }

        QPlainTextEdit::dragMoveEvent( event );
    }

    void TextEdit::dropEvent( QDropEvent* event )
    {
        bool doF = false;
        if( event->mimeData()->hasUrls() )
        {
            const QList<QUrl> urls = event->mimeData()->urls();
            for( int i = 0; i < urls.size(); ++i )
            {
                const QUrl& url = urls.at( i );
                if( url.isLocalFile() )
                {
                    MainWindow* win = qobject_cast<MainWindow*>( window() );
                    if( win )
                    {
                        win->openFile( url.toLocalFile() );
                        doF = true;
                    }
                }
            }
        }
        if( doF )
        {
            event->acceptProposedAction();
            return;
        }

        QPlainTextEdit::dropEvent( event );
    }

    void TextEdit::updateViewportMargins( const bool force )
    {
        if( m_lineNumberVisible )
        {
            const int lineNumberWidth =
                qMax( QString( "%1" ).arg( blockCount() ).length(), 4 ) + 1;
            if( force || ( lineNumberWidth != m_lineNumberWidth ) )
            {
                QFontMetrics fontMetrics( document()->defaultFont() );
                // TODO: Proportional Font
                const int lineNumberPixelWidth =
                    fontMetrics.width( "0" ) * lineNumberWidth;
                setViewportMargins( lineNumberPixelWidth, 0, 0, 0 );
                m_lineNumberWidth = lineNumberWidth;
                m_lineNumberPixelWidth = lineNumberPixelWidth;
            }
        }
        else
        {
            setViewportMargins( 0, 0, 0, 0 );
        }
    }

    void TextEdit::updateLineNumberWidgetGeometry( void )
    {
        if( !m_lineNumberWidget )
        {
            return;
        }

        QRect geometry = contentsRect();
        geometry.setWidth( m_lineNumberPixelWidth );
        m_lineNumberWidget->setGeometry( geometry );
    }

    void TextEdit::updateTabStopWidthBySpace( void )
    {
        QFontMetrics fontMetrics( document()->defaultFont() );
        setTabStopWidth( fontMetrics.width( " " ) * m_tabStopWidthBySpace );
    }

    void TextEdit::updateExtraSelections()
    {
        QList<QTextEdit::ExtraSelection> extraSelections;

        if( !textCursor().hasSelection() )
        {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground( QColor( Qt::cyan ).lighter( 180 ) );
            selection.format.setProperty( QTextFormat::FullWidthSelection, true );
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append( selection );
        }

        if( ( m_coBracePos[0] >= 0 ) && ( m_coBracePos[1] >= 0 ) )
        {
            QTextEdit::ExtraSelection selection;
            selection.format.setForeground( Qt::red );
            selection.cursor = textCursor();
            if( qAbs( m_coBracePos[0] - m_coBracePos[1] ) == 1 )
            {
                selection.cursor.setPosition( qMin( m_coBracePos[0], m_coBracePos[1] ) );
                selection.cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2 );
                extraSelections.append( selection );
            }
            else
            {
                selection.cursor.setPosition( m_coBracePos[0] );
                selection.cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
                extraSelections.append( selection );

                selection.cursor.setPosition( m_coBracePos[1] );
                selection.cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
                extraSelections.append( selection );
            }
        }

        setExtraSelections( extraSelections );
    }

    void TextEdit::updateCoBracePos()
    {
        const int curPos = m_coBracePos[0];
        m_coBracePos[0] = -1;
        m_coBracePos[1] = -1;

        QTextDocument* document = this->document();
        const QChar brace = document->characterAt( curPos );
        int step;
        QChar coBrace;
        if( brace == '(' )
        {
            step = 1;
            coBrace = ')';
        }
        else if( brace == ')' )
        {
            step = -1;
            coBrace = '(';
        }
        else if( brace == '{' )
        {
            step = 1;
            coBrace = '}';
        }
        else if( brace == '}' )
        {
            step = -1;
            coBrace = '{';
        }
        else if( brace == '[' )
        {
            step = 1;
            coBrace = ']';
        }
        else if( brace == ']' )
        {
            step = -1;
            coBrace = '[';
        }
        else if( brace == '<' )
        {
            step = 1;
            coBrace = '>';
        }
        else if( brace == '>' )
        {
            step = -1;
            coBrace = '<';
        }
        else
        {
            return;
        }

        if( isPartOfString( curPos ) )
        {
            return;
        }

        int nesting = 0;
        for( int pos = curPos + step; ; pos += step )
        {
            const QChar ch = document->characterAt( pos );
            if( ch.isNull() )
            {
                break;
            }
            else if( ch == brace )
            {
                if( !isPartOfString( pos ) )
                {
                    ++nesting;
                }
            }
            else if( ch == coBrace )
            {
                if( !isPartOfString( pos ) )
                {
                    if( nesting > 0 )
                    {
                        --nesting;
                    }
                    else
                    {
                        m_coBracePos[0] = curPos;
                        m_coBracePos[1] = pos;
                        break;
                    }
                }
            }
        }
    }

    void TextEdit::drawLineNumber( void )
    {
        if( !m_lineNumberWidget )
        {
            return;
        }

        const QRect rect = m_lineNumberWidget->rect();

        QPainter painter( m_lineNumberWidget );
        painter.setFont( document()->defaultFont() );

        painter.setPen( Qt::lightGray );
        painter.drawLine( rect.topRight(), rect.bottomRight() );

        painter.setPen( m_lineNumberWidget->palette().color( QPalette::WindowText ) );

        for( QTextBlock block = firstVisibleBlock();
             block.isValid();
             block = block.next() )
        {
            const QRectF blockRect =
                blockBoundingGeometry( block ).translated( contentOffset() );
            if( blockRect.top() > ( qreal )rect.bottom() )
            {
                break;
            }

            const QTextLayout* layout = block.layout();
            if( !layout || ( layout->lineCount() < 1 ) )
            {
                continue;
            }

            const QTextLine line = layout->lineAt( 0 );
            const QRectF lineRect = line.rect().translated( blockRect.topLeft() );

            QRectF textRect = lineRect;
            textRect.setLeft( rect.left() );
            textRect.setRight( rect.right() - 1 );

            painter.drawText(
                textRect,
                Qt::AlignRight | Qt::AlignBottom | Qt::TextSingleLine,
                QString( "%1" ).arg( block.blockNumber() + 1 ) );
        }
    }

    void TextEdit::drawEOF( void )
    {
        QTextBlock block = document()->lastBlock();
        const QRectF blockRect =
            blockBoundingGeometry( block ).translated( contentOffset() );

        const QTextLayout* layout = block.layout();
        if( !layout || ( layout->lineCount() < 1 ) )
        {
            return;
        }

        const QTextLine line = layout->lineAt( layout->lineCount() - 1 );
        const QRectF lineRect = line.naturalTextRect().translated( blockRect.topLeft() );

        QRectF textRect = lineRect;
        textRect.moveLeft( textRect.right() );

        QPainter painter( viewport() );
        painter.setFont( document()->defaultFont() );
        painter.setPen( Qt::lightGray );
        painter.drawText(
            textRect,
            Qt::AlignLeft | Qt::AlignBottom | Qt::TextSingleLine | Qt::TextDontClip,
            tr( "[EOF]" ) );
    }

    void TextEdit::drawNewlineCharacter( void )
    {
        const QRect rect = viewport()->rect();

        QPainter painter( viewport() );
        painter.setFont( document()->defaultFont() );
        painter.setPen( Qt::lightGray );

        for( QTextBlock block = firstVisibleBlock();
             block.isValid() && ( block != document()->lastBlock() );
             block = block.next() )
        {
            const QRectF blockRect =
                blockBoundingGeometry( block ).translated( contentOffset() );
            if( blockRect.top() > ( qreal )rect.bottom() )
            {
                break;
            }

            const QTextLayout* layout = block.layout();
            if( !layout || ( layout->lineCount() < 1 ) )
            {
                continue;
            }

            const QTextLine line = layout->lineAt( layout->lineCount() - 1 );
            const QRectF lineRect = line.naturalTextRect().translated( blockRect.topLeft() );

            QRectF textRect = lineRect;
            textRect.moveLeft( textRect.right() );

            painter.drawText(
                textRect,
                Qt::AlignLeft | Qt::AlignBottom | Qt::TextSingleLine | Qt::TextDontClip,
                QChar( 0x2193 ) );
        }
    }

    void TextEdit::drawTabCharacter( void )
    {
        const QRect rect = viewport()->rect();

        QPainter painter( viewport() );
        painter.setFont( document()->defaultFont() );
        painter.setPen( Qt::lightGray );

        for( QTextBlock block = firstVisibleBlock();
             block.isValid();
             block = block.next() )
        {
            const QRectF blockRect =
                blockBoundingGeometry( block ).translated( contentOffset() );
            if( blockRect.top() > ( qreal )rect.bottom() )
            {
                break;
            }

            const QTextLayout* layout = block.layout();
            if( !layout )
            {
                continue;
            }

            const QString text = block.text();
            const int len = text.length();
            for( int i = 0; i < len; ++i )
            {
                if( text[i] == '\t' )
                {
                    int cursorPos = i;
                    QTextLine line = layout->lineForTextPosition( cursorPos );
                    if( line.isValid() )
                    {
                        const qreal x = line.cursorToX( &cursorPos );
                        QRectF textRect = line.rect().translated( blockRect.topLeft() );
                        textRect.moveLeft( textRect.left() + x );
                        painter.drawText(
                            textRect,
                            Qt::AlignLeft | Qt::AlignBottom | Qt::TextSingleLine | Qt::TextDontClip,
                            QChar( 0x21C0 ) );
                    }
                }
            }
        }
    }

    void TextEdit::drawGuideLine()
    {
        const QFontMetrics fontMetrics( document()->defaultFont(), viewport() );
        const int width = fontMetrics.width( "W" ) * 80 + document()->documentMargin();

        QPainter painter( viewport() );
        painter.setPen( QPen( Qt::lightGray, 0, Qt::DotLine ) );
        painter.drawLine( QPoint( width, 0 ), QPoint( width, height() - 1 ) );
    }

    void TextEdit::indent( void )
    {
        QTextCursor textCursor = this->textCursor();
        int anchor = textCursor.anchor();
        int position = textCursor.position();
        QTextBlock anchorBlock = document()->findBlock( anchor );
        QTextBlock positionBlock = document()->findBlock( position );
        if( anchorBlock != positionBlock )
        {
            if( anchor < position )
            {
                anchor = anchorBlock.position();

                textCursor.beginEditBlock();
                QTextBlock block = anchorBlock;
                while( block.isValid() )
                {
                    if( block == positionBlock )
                    {
                        textCursor.setPosition( position );

                        if( textCursor.positionInBlock() == 0 )
                        {
                            break;
                        }
                        else
                        {
                            textCursor.movePosition( QTextCursor::StartOfBlock );
                        }
                    }
                    else
                    {
                        textCursor.setPosition( block.position() );
                    }

                    textCursor.insertText( QString( tabStopWidthBySpace(), ' ' ) );
                    position += tabStopWidthBySpace();

                    if( block == positionBlock )
                    {
                        break;
                    }

                    block = block.next();
                }
                textCursor.endEditBlock();

                textCursor.setPosition( anchor );
                textCursor.setPosition( position, QTextCursor::KeepAnchor );
                if( textCursor.positionInBlock() != 0 )
                {
                    textCursor.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
                }
                setTextCursor( textCursor );
            }
            else
            {
                position = positionBlock.position();

                textCursor.beginEditBlock();
                QTextBlock block = anchorBlock;
                while( block.isValid() )
                {
                    if( block == anchorBlock )
                    {
                        textCursor.setPosition( anchor );

                        if( textCursor.positionInBlock() == 0 )
                        {
                            block = block.previous();
                            continue;
                        }
                        else
                        {
                            textCursor.movePosition( QTextCursor::StartOfBlock );
                        }
                    }
                    else
                    {
                        textCursor.setPosition( block.position() );
                    }

                    textCursor.insertText( QString( tabStopWidthBySpace(), ' ' ) );
                    anchor += tabStopWidthBySpace();

                    if( block == positionBlock )
                    {
                        break;
                    }

                    block = block.previous();
                }
                textCursor.endEditBlock();

                textCursor.setPosition( anchor );
                if( textCursor.positionInBlock() != 0 )
                {
                    textCursor.movePosition( QTextCursor::EndOfBlock );
                }
                textCursor.setPosition( position, QTextCursor::KeepAnchor );
                setTextCursor( textCursor );
            }
        }
        else
        {
            textCursor.beginEditBlock();
            if( textCursor.hasSelection() )
            {
                textCursor.deleteChar();
            }
            const int n = tabStopWidthBySpace() - ( textCursor.positionInBlock() % tabStopWidthBySpace() );
            textCursor.insertText( QString( n, ' ' ) );
            textCursor.endEditBlock();
            setTextCursor( textCursor );
        }
    }

    void TextEdit::reverseIndent( void )
    {
        QTextCursor textCursor = this->textCursor();
        int anchor = textCursor.anchor();
        int position = textCursor.position();
        QTextBlock anchorBlock = document()->findBlock( anchor );
        QTextBlock positionBlock = document()->findBlock( position );
        if( anchorBlock != positionBlock )
        {
            if( anchor < position )
            {
                anchor = anchorBlock.position();

                textCursor.beginEditBlock();
                QTextBlock block = anchorBlock;
                while( block.isValid() )
                {
                    if( block == positionBlock )
                    {
                        textCursor.setPosition( position );

                        if( textCursor.positionInBlock() == 0 )
                        {
                            break;
                        }
                        else
                        {
                            textCursor.movePosition( QTextCursor::StartOfBlock );
                        }
                    }
                    else
                    {
                        textCursor.setPosition( block.position() );
                    }

                    int count = tabStopWidthBySpace();
                    while( count > 0 )
                    {
                        const QChar ch = document()->characterAt( textCursor.position() );
                        if( ch == ' ' )
                        {
                            textCursor.deleteChar();
                            --position;
                            --count;
                        }
                        else if( ch == '\t' )
                        {
                            textCursor.deleteChar();
                            --position;
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }

                    if( block == positionBlock )
                    {
                        break;
                    }

                    block = block.next();
                }
                textCursor.endEditBlock();

                textCursor.setPosition( anchor );
                textCursor.setPosition( position, QTextCursor::KeepAnchor );
                if( textCursor.positionInBlock() != 0 )
                {
                    textCursor.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
                }
                setTextCursor( textCursor );
            }
            else
            {
                position = positionBlock.position();

                textCursor.beginEditBlock();
                QTextBlock block = anchorBlock;
                while( block.isValid() )
                {
                    if( block == anchorBlock )
                    {
                        textCursor.setPosition( anchor );

                        if( textCursor.positionInBlock() == 0 )
                        {
                            block = block.previous();
                            continue;
                        }
                        else
                        {
                            textCursor.movePosition( QTextCursor::StartOfBlock );
                        }
                    }
                    else
                    {
                        textCursor.setPosition( block.position() );
                    }

                    int count = tabStopWidthBySpace();
                    while( count > 0 )
                    {
                        const QChar ch = document()->characterAt( textCursor.position() );
                        if( ch == ' ' )
                        {
                            textCursor.deleteChar();
                            --anchor;
                            --count;
                        }
                        else if( ch == '\t' )
                        {
                            textCursor.deleteChar();
                            --anchor;
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }

                    if( block == positionBlock )
                    {
                        break;
                    }

                    block = block.previous();
                }
                textCursor.endEditBlock();

                textCursor.setPosition( anchor );
                if( textCursor.positionInBlock() != 0 )
                {
                    textCursor.movePosition( QTextCursor::EndOfBlock );
                }
                textCursor.setPosition( position, QTextCursor::KeepAnchor );
                setTextCursor( textCursor );
            }
        }
    }

    bool TextEdit::isPartOfString( const int pos )const
    {
        QTextBlock block = document()->findBlock( pos );
        if( !block.isValid() )
        {
            return false;
        }

        const QTextLayout* layout = block.layout();
        if( !layout )
        {
            return false;
        }

        const QList<QTextLayout::FormatRange> additionalFormats =
            layout->additionalFormats();
        for( int i = 0; i < additionalFormats.size(); ++i )
        {
            const QTextLayout::FormatRange& range = additionalFormats.at( i );
            if( !range.format.boolProperty( QTextFormat::UserProperty + 1 ) )
            {
                continue;
            }

            if( ( range.start <= pos - block.position() ) &&
                ( range.start + range.length > pos - block.position() ) )
            {
                return true;
            }
        }

        return false;
    }

    void TextEdit::inputCompletion( void )
    {
        QTextCursor textCursor = this->textCursor();
        if( textCursor.hasSelection() )
        {
            return;
        }
        const int curPos = textCursor.position();

        const QRegExp partOfWordExp( "[a-z0-9_]", Qt::CaseInsensitive );
        QString text;
        for( int pos = curPos - 1; ; --pos )
        {
            const QChar ch = document()->characterAt( pos );
            if( ch.isNull() || !partOfWordExp.exactMatch( QString( ch ) ) )
            {
                break;
            }

            text.prepend( ch );
        }
        if( text.isEmpty() )
        {
            return;
        }

        QMap<QString, int> hitStrAndDistMap;

        const QRegExp wordExp( text + partOfWordExp.pattern() + '+', Qt::CaseInsensitive );
        textCursor.movePosition( QTextCursor::Start );
        while( true )
        {
            textCursor = document()->find( wordExp, textCursor, QTextDocument::FindWholeWords );
            if( textCursor.isNull() )
            {
                break;
            }

            if( textCursor.anchor() != curPos - text.length() )
            {
                int dist;
                if( textCursor.anchor() < curPos )
                {
                    dist = qAbs( curPos - text.length() - textCursor.position() );
                }
                else
                {
                    dist = qAbs( curPos - text.length() - textCursor.anchor() );
                }

                const QString str = textCursor.selectedText();
                QMap<QString, int>::iterator itr = hitStrAndDistMap.find( str );
                if( itr != hitStrAndDistMap.end() )
                {
                    if( dist < itr.value() )
                    {
                        itr.value() = dist;
                    }
                }
                else
                {
                    hitStrAndDistMap.insert( str, dist );
                }
            }
        }

        if( !hitStrAndDistMap.isEmpty() )
        {
            QVector<QPair<int, QString> > hitDistAndStrArr;
            hitDistAndStrArr.reserve( hitStrAndDistMap.size() );
            for( QMap<QString, int>::const_iterator itr = hitStrAndDistMap.begin();
                 itr != hitStrAndDistMap.end(); ++itr )
            {
                hitDistAndStrArr.push_back( QPair<int, QString>( itr.value(), itr.key() ) );
            }
            qSort( hitDistAndStrArr.begin(), hitDistAndStrArr.end() );

            if( !m_inputCompletionList )
            {
                m_inputCompletionList = new QListWidget( viewport() );
                m_inputCompletionList->setItemDelegate(
                    new InputCompletionItemDelegate( m_inputCompletionList ) );
                m_inputCompletionList->setFont( document()->defaultFont() );
                m_inputCompletionList->setFocusPolicy( Qt::NoFocus );
            }

            m_inputCompletionList->clear();
            for( int i = 0; i < hitDistAndStrArr.size(); ++i )
            {
                m_inputCompletionList->addItem( hitDistAndStrArr.at( i ).second );
            }
            m_inputCompletionList->setCurrentRow( 0 );

            textCursor = this->textCursor();
            textCursor.movePosition(
                QTextCursor::PreviousCharacter,
                QTextCursor::KeepAnchor,
                text.length() );
            const QRect cursorRect = this->cursorRect( textCursor );

            QRect geometry( cursorRect.bottomLeft() + QPoint( 0, 1 ), m_inputCompletionList->sizeHint() );
            if( viewport()->mapToGlobal( geometry.bottomLeft() ).y() > mapToGlobal( contentsRect().bottomLeft() ).y() )
            {
                geometry.moveBottom( cursorRect.top() - 1 );
            }
            m_inputCompletionList->setGeometry( geometry );

            m_inputCompletionList->show();
        }
    }

    bool TextEdit::isInputCompletionVisible( void )const
    {
        return m_inputCompletionList && m_inputCompletionList->isVisible();
    }

    void TextEdit::applyInputCompletion( void )
    {
        if( !m_inputCompletionList )
        {
            return;
        }

        const QListWidgetItem* item = m_inputCompletionList->currentItem();
        if( !item )
        {
            return;
        }

        const QString str = item->text();
        if( str.isEmpty() )
        {
            return;
        }

        QTextCursor textCursor = this->textCursor();
        if( textCursor.hasSelection() )
        {
            return;
        }
        const int curPos = textCursor.position();

        const QRegExp partOfWordExp( "[a-z0-9_]", Qt::CaseInsensitive );
        QString text;
        for( int pos = curPos - 1; ; --pos )
        {
            const QChar ch = document()->characterAt( pos );
            if( ch.isNull() || !partOfWordExp.exactMatch( QString( ch ) ) )
            {
                break;
            }

            text.prepend( ch );
        }
        if( text.isEmpty() )
        {
            return;
        }

        textCursor.movePosition(
            QTextCursor::PreviousCharacter,
            QTextCursor::KeepAnchor,
            text.length() );
        textCursor.beginEditBlock();
        textCursor.deleteChar();
        textCursor.insertText( str );
        textCursor.endEditBlock();
        setTextCursor( textCursor );
    }

    void TextEdit::autoIndent( void )
    {
        QTextCursor cursor = textCursor();
        if( cursor.positionInBlock() == 0 )
        {
            QTextBlock prevBlock = cursor.block().previous();
            if( prevBlock.isValid() )
            {
                const QString prevText = prevBlock.text();
                QString indent;
                for( QString::const_iterator itr = prevText.begin();
                     itr != prevText.end();
                     ++itr )
                {
                    if( ( *itr == QChar( ' ' ) ) ||
                        ( *itr == QChar( '\t' ) ) )
                    {
                        indent += *itr;
                    }
                    else
                    {
                        break;
                    }
                }
                if( !indent.isEmpty() )
                {
                    cursor.joinPreviousEditBlock();
                    cursor.insertText( indent );
                    cursor.endEditBlock();
                    setTextCursor( cursor );
                }
            }
        }
    }

    QTextCursor TextEdit::_findNext( const QString& text, const QTextDocument::FindFlags flags )const
    {
        if( text.isEmpty() )
        {
            return QTextCursor();
        }
        else
        {
            return document()->find( text, this->textCursor(), flags );
        }
    }

    QTextCursor TextEdit::_findNext( const QRegExp& expr, const QTextDocument::FindFlags flags )const
    {
        if( expr.isEmpty() )
        {
            return QTextCursor();
        }
        else
        {
            return document()->find( expr, this->textCursor(), flags );
        }
    }

    QTextCursor TextEdit::_findPrevious( const QString& text, const QTextDocument::FindFlags flags )const
    {
        if( text.isEmpty() )
        {
            return QTextCursor();
        }
        else
        {
            return document()->find( text, this->textCursor(), flags | QTextDocument::FindBackward );
        }
    }

    QTextCursor TextEdit::_findPrevious( const QRegExp& expr, const QTextDocument::FindFlags flags )const
    {
        if( expr.isEmpty() )
        {
            return QTextCursor();
        }
        else
        {
            return document()->find( expr, this->textCursor(), flags | QTextDocument::FindBackward );
        }
    }

    void TextEdit::onFontChanged( void )
    {
        updateGeometry();
        updateViewportMargins( true );
        updateLineNumberWidgetGeometry();
        updateTabStopWidthBySpace();
    }

    void TextEdit::onCursorPositionChanged()
    {
        if( isInputCompletionVisible() )
        {
            m_inputCompletionList->hide();
        }

        QTextCursor textCursor = this->textCursor();
        if( textCursor.hasSelection() || textCursor.atBlockEnd() )
        {
            m_coBracePos[0] = -1;
        }
        else
        {
            m_coBracePos[0] = textCursor.position();
        }
        updateCoBracePos();
        updateExtraSelections();
    }

    void TextEdit::onSelectionChanged()
    {
        updateExtraSelections();
    }

    void TextEdit::onTextChanged()
    {
        QTextCursor textCursor = this->textCursor();
        if( textCursor.hasSelection() || textCursor.atBlockEnd() )
        {
            m_coBracePos[0] = -1;
        }
        else
        {
            m_coBracePos[0] = textCursor.position();
        }
        updateCoBracePos();
        updateExtraSelections();
    }
}
