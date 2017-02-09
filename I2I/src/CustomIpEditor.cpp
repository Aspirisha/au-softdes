#include "CustomIpEditor.h"

#include <QRegExpValidator>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QHBoxLayout>

CustomLineEdit::CustomLineEdit(const QString & contents, QWidget *parent) :
    QLineEdit(contents, parent), selectOnMouseRelease(false)
{
    QRegExp regexp("^(25[0-5]|2[0-4]\\d|[01]\\d\\d|\\d\\d|\\d)$");
    QRegExpValidator *validator = new QRegExpValidator(regexp, this);
    setValidator(validator);
}

void CustomLineEdit::jumpIn()
{
    setFocus();

    selectOnMouseRelease = false;
    selectAll();
}

void CustomLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    selectOnMouseRelease = true;
}

void CustomLineEdit::keyPressEvent(QKeyEvent * event)
{
    int key = event->key();
    int cursorPos = cursorPosition();

    // Jump forward by Space
    if (key == Qt::Key_Space) {
        emit jumpForward();
        event->accept();
        return;
    }

    // Jump Backward only from 0 cursor position
    if (cursorPos == 0) {
        if ((key == Qt::Key_Left) || (key == Qt::Key_Backspace)) {
            emit jumpBackward();
            event->accept();
            return;
        }
    }

    // Jump forward from last postion by right arrow
    if (cursorPos == text().count()) {
        if (key == Qt::Key_Right) {
            emit jumpForward();
            event->accept();
            return;
        }
    }

    // After key is placed cursor has new position
    QLineEdit::keyPressEvent(event);
    int freshCurPos = cursorPosition();

    auto can_have_third_digit = [](QString text) {
        return text.at(0) < '2' || (text.at(0) == '2' && text.at(1) < '6');
    };

    if (freshCurPos == 3 || (freshCurPos == 2 && !can_have_third_digit(text())))
        emit jumpForward();
}

void CustomLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
    if(!selectOnMouseRelease)
        return;

    selectOnMouseRelease = false;
    selectAll();

    QLineEdit::mouseReleaseEvent(event);
}

void makeCommonStyle(QLineEdit* line) {
    line->setContentsMargins(0, 0, 0, 0);
    line->setAlignment(Qt::AlignCenter);
    line->setStyleSheet("QLineEdit { border: 0px none; }");
    line->setFrame(false);
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

QLineEdit* makeIpSpliter() {
    QLineEdit *spliter = new QLineEdit(".");
    makeCommonStyle(spliter);

    spliter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spliter->setMaximumWidth(10);
    spliter->setReadOnly(true);
    spliter->setFocusPolicy(Qt::NoFocus);
    return spliter;
}

CustomIpEditor::CustomIpEditor(QWidget *parent) :
    QFrame(parent), filledBitMask(0)
{
    setContentsMargins(0, 0, 0, 0);
    setStyleSheet("QFrame { background-color: white;  border: 1px solid black; border-radius: 2px; }");

    QList <CustomLineEdit *>  lines;
    QList <CustomLineEdit *>::iterator linesIterator;

    lines.append(new CustomLineEdit);
    lines.append(new CustomLineEdit);
    lines.append(new CustomLineEdit);
    lines.append(new CustomLineEdit);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    for (size_t i = 0; i < lines.size(); i++) {
        auto line = lines.at(i);
        makeCommonStyle(line);
        mainLayout->addWidget(line);

        if (i != lines.size() - 1) {
            connect(line, &CustomLineEdit::jumpForward,
                    lines.at(i + 1), &CustomLineEdit::jumpIn);
            mainLayout->addWidget(makeIpSpliter());
        }
        if (i != 0) {
            connect(line, &CustomLineEdit::jumpBackward,
                    lines.at(i - 1), &CustomLineEdit::jumpIn);
        }

        connect(line, &CustomLineEdit::textEdited, [this, i](const QString &s) {
            this->onTextEdited(i, s);
        });
    }
}

QString CustomIpEditor::getText() const
{
    QString result;
    for (QObject *c : children()) {
        QLineEdit *edit = dynamic_cast<QLineEdit*>(c);
        if (!edit) continue;
        result += edit->text();
    }
    return result;
}

void CustomIpEditor::onTextEdited(int i, const QString &s)
{
    char oldMask = filledBitMask;
    filledBitMask &= ~(1 << i);
    filledBitMask |= ((s.size() != 0) << i);

    if (filledBitMask != oldMask && filledBitMask == 0xf)
        emit fillChanged(true);
    else if (filledBitMask != oldMask && oldMask == 0xf)
        emit fillChanged(false);
}
