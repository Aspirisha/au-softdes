#include <QByteArray>
#include "networkmanagertest.h"
#include "socketinterface.h"
#include "User.h"
#include "Message.h"

using namespace i2imodel;
using namespace i2inet;


NetworkManagerTest::NetworkManagerTest()
{
}

void NetworkManagerTest::simpleInteractionWithI2I() {
    QSignalSpy spy(otherManager.data(), SIGNAL(peerGreeted(QSharedPointer<i2imodel::Chat>)));
    QSignalSpy spy2(otherManager.data(), SIGNAL(messageReceived(i2imodel::Message)));

    Message receievedMessage("", QDateTime::currentDateTime(), 0);
    QObject::connect(otherManager.data(), &NetworkManager::messageReceived, [&receievedMessage](Message m) {
        receievedMessage = m;
    });

    myManager->sendMessage(peerUser->getId(), "Hello!");
    for (int waitDelay = 500; waitDelay > 0 && spy.count() == 0; waitDelay -= 100) {
        QTest::qWait(100);
    }
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy2.count(), 1);

    QCOMPARE(receievedMessage.getText(), QString("Hello!"));
}

void NetworkManagerTest::testChangeNameWithI2I()
{
    QSignalSpy spy(otherManager.data(), SIGNAL(brodcastMessage(i2inet::BroadcastMessage)));

    ownUser->updateLogin("Vovochka");
    myManager->ownLoginChanged();
    for (int waitDelay = 500; waitDelay > 0 && spy.count() == 0; waitDelay -= 100) {
        QTest::qWait(100);
    }
    QCOMPARE(spy.count(), 1);

}

void NetworkManagerTest::init()
{
    static int counter = 1;
    logger()->debug(QString("================== TEST #%1 ========================").arg(counter++));

    myActiveConnection.reset(new TcpClientMock(true));
    myPassiveConnection.reset(new TcpClientMock(false));
    otherActiveConnection.reset(new TcpClientMock(true));
    otherPassiveConnection.reset(new TcpClientMock(false));
    myBroadcast.reset(new UdpSocketMock);
    otherBroadcast.reset(new UdpSocketMock);

    TcpClientMock::connectTwoSockets(myActiveConnection.data(), otherPassiveConnection.data());
    UdpSocketMock::connectTwoSockets(myBroadcast.data(), otherBroadcast.data());


    // Test case 1: we try to connect to I2I chat
    ownUser.reset(new User("Vasya",QHostAddress("127.0.0.1").toIPv4Address(), 1234));
    peerUser.reset(new User("Petya",QHostAddress("127.0.0.1").toIPv4Address(), 3673));
    myServer.reset(new TcpServerMock(myPassiveConnection.data()));
    otherServer.reset(new TcpServerMock(otherPassiveConnection.data()));

    QObject::connect(myActiveConnection.data(), SIGNAL(iAmKnocking()), otherServer.data(), SIGNAL(newConnection()));
    QObject::connect(otherActiveConnection.data(), SIGNAL(iAmKnocking()), myServer.data(), SIGNAL(newConnection()));

    myManager.reset(new NetworkManager(ownUser, myServer, [this](QObject *) {
       return myActiveConnection.data();
    }, myBroadcast));

    otherManager.reset(new NetworkManager(peerUser, otherServer, [this](QObject *) {
       return otherActiveConnection.data();
    }, otherBroadcast));
}
