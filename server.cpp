#include "server.h"

#include <QDateTime>
#include <QFile>
#include <QPixmap>
#include <QSaveFile>

Server::Server()
{
    if(this->listen(QHostAddress::Any, 8001))
    {

    }
    else
    {

    }
}


void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    Sockets.push_back(socket);
}

void Server::slotReadyRead()
{
    socket = (QTcpSocket *)sender();
    QString request = socket->readAll();
    qDebug() << request;
    QJsonDocument jsonRequest = QJsonDocument::fromJson(request.toUtf8());
    QJsonObject jsonObject = jsonRequest.object();
    QJsonObject jsonResponce;
    if (jsonObject["type_request"].toString() == "CONNECT") {
        // User connected
        jsonResponce["type_responce"] = "CREATE";
        if (users.find(jsonObject["name"].toString()) == users.end()) {
            users.insert(jsonObject["name"].toString());
            jsonResponce["answer"] = true;
            emit newUser(jsonObject["name"].toString());
        }
        else {
            jsonResponce["answer"] = false;
        }
    }
    else if (jsonObject["type_request"].toString() == "NEW_TASK") {
        // User ask me that you could to give new task
        jsonResponce["type_responce"] = "NEW_TASKS";
        QJsonArray tasks_list;
        QString name = jsonObject["name"].toString();
        while(tasks[name].isEmpty()){
            tasks_list.push_back(tasks[name].pop());
        }
        jsonResponce["tasks"] = tasks_list;
    }
    else if (jsonObject["type_request"].toString() == "SEND_FILE_SOLUTION") {
        // User send me image
        QString path = jsonObject["name"].toString()+
                jsonObject["task_name"].toString()+
                jsonObject["task_type"].toString()+
                QDateTime::currentMSecsSinceEpoch();
        // todo: дописать
        emit newSolution(path);
    }
    else if (jsonObject["type_request"].toString() == "SEND_SOLUTION") {
        // User send me text solution
        QString path = jsonObject["name"].toString()+
                jsonObject["task_name"].toString()+
                jsonObject["task_type"].toString()+
                QDateTime::currentMSecsSinceEpoch();
        // todo: дописать
        emit newSolution(path);
    }
    else if (jsonObject["type_request"].toString() == "GET_TASK") {
        // User ask me a task
    }

    QJsonDocument doc(jsonResponce);
    QString strResponce(doc.toJson(QJsonDocument::Compact));

    socket->write(strResponce.toLocal8Bit());

    socket->close();
}


