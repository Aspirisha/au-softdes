#include "Message.h"

namespace i2imodel {

QDateTime Message::getDate() const
{
    return date;
}

QString Message::getText() const
{
    return text;
}

userid_t Message::getAuthor() const
{
    return authorId;
}

QtJson::JsonObject Message::toJson() const
{
    QtJson::JsonObject json;

    json["text"] = text;
    json["date"] = date.toString();
    json["authorId"] = authorId;
    return json;
}

Message Message::fromJson(const QMap<QString, QVariant> &json)
{
    QString text = json["text"].toString();
    QDateTime date = json["date"].toDateTime();
    userid_t authorId = json["authorId"].toULongLong();

    return Message(text, date, authorId);
}

}
