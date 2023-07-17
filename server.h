#pragma once
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QStack>


class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server();
    QTcpSocket *socket;
    QSet<QString> users;
    QMap<QString, QStack<QString>> tasks;
    void rata();

private:
    QVector <QTcpSocket *> Sockets;
    QByteArray Data;
    void SendToClient();

public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

signals:
    void newUser(QString User);
    void newSolution(QString User);

};

#endif // SERVER_H
