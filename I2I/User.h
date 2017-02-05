#ifndef USER_H
#define USER_H

#include <QHostAddress>
#include "ModelCommon.h"
#include "qt-json/json.h"

namespace i2imodel {
class AbstractUser {
public:
    virtual QString getLogin() const = 0;
    virtual userid_t getId() const = 0;
    virtual ~AbstractUser() {}
};

class User : public AbstractUser
{
public:
    User(QString login, quint32 ip, quint16 port) : login(login), ip(ip), port(port) {}

    QString getLogin() const override { return login; }
    userid_t getId() const override { return (static_cast<userid_t>(ip) << 16) + port; }
    QtJson::JsonObject toJson() const;
    static User* fromJson(const QString &obj);
    static User* fromJson(const QtJson::JsonObject &obj);
private:
    const QString login;
    const quint32 ip;
    const quint16 port;
};
}
#endif // USER_H
