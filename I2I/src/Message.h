#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include "User.h"

namespace i2imodel {

class Message
{
public:
    Message(QString text, QDateTime date, userid_t authorId)
        : text(text), date(date), authorId(authorId) {}
    QDateTime getDate() const;
    QString getText() const;
    userid_t getAuthor() const;

    QtJson::JsonObject toJson() const;
    static Message fromJson(const QMap<QString, QVariant> & json);
private:
    QString text;
    QDateTime date;
    userid_t authorId;
};

}

#endif // MESSAGE_H
