#include "User.h"

namespace i2imodel {

QtJson::JsonObject User::toJson() const
{
    QtJson::JsonObject serialized;
    serialized["login"] = login;
    serialized["ip"] = ip;
    serialized["port"] = port;
    return serialized;
}

User *User::fromJson(const QString &json)
{
    bool ok;
    QtJson::JsonObject result = QtJson::parse(json, ok).toMap();
    if(!ok) {
        qDebug("An error occurred during parsing");
        return nullptr;
    }

    return fromJson(result);
}

User *User::fromJson(const QtJson::JsonObject &obj)
{
    return new User(obj["login"].toString(), obj["ip"].toUInt(), obj["port"].toUInt());
}
}
