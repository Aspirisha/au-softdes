#include "User.h"

namespace i2imodel {

QtJson::JsonObject User::toJson() const
{
    QtJson::JsonObject json;

    json["login"] = login;
    json["ip"] = ip;
    json["port"] = port;
    return json;
}

QByteArray User::serialize() const
{
    return QtJson::serialize(toJson());
}

QSharedPointer<User> User::fromJson(const QString &json)
{
    bool ok;
    QtJson::JsonObject result = QtJson::parse(json, ok).toMap();
    if(!ok) {
        qDebug("An error occurred during parsing");
        return {};
    }

    return fromJson(result);
}

QSharedPointer<User> User::fromJson(const QMap<QString, QVariant> &obj)
{
    return QSharedPointer<User>{new User(obj["login"].toString(), obj["ip"].toUInt(), obj["port"].toUInt())};
}
}
