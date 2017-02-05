#ifndef CUSTOMIPEDIT_H
#define CUSTOMIPEDIT_H

#include <QFrame>
#include <QLineEdit>

class CustomIpEditor : public QFrame
{
    Q_OBJECT
public:
    explicit CustomIpEditor(QWidget *parent = 0);
    bool isFilled() const;
    virtual ~CustomIpEditor() {}
signals:
    void fillChanged(bool filled);
protected slots:
    void onTextEdited(int i, const QString &s);
private:
    char filledBitMask;
};


class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(const QString & contents = "", QWidget *parent = 0);
    virtual ~CustomLineEdit() {}

signals:
    void jumpForward();
    void jumpBackward();

public slots:
    void jumpIn();

protected:
    virtual void focusInEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    bool selectOnMouseRelease;
};

#endif // CUSTOMIPEDIT_H
