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
}
