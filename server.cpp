#include "server.h"

#include <QDateTime>
#include <QFile>
#include <QPixmap>
#include <QSaveFile>
#include <QImage>
#include <QDir>
#include <QBuffer>

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
}


void Server::saveByteArrayAsJpg(const QByteArray& byteArray, const QString& filePath)
{
    QImage image;
    if (image.loadFromData(byteArray)) {
        // Ensure the file path has ".jpg" extension
        QString filePathWithExtension = filePath;
        if (!filePath.endsWith(".jpg", Qt::CaseInsensitive))
            filePathWithExtension += ".jpg";

        if (image.save(filePathWithExtension, "JPG")) {
            qDebug() << "Image saved as .jpg file:" << filePathWithExtension;
        } else {
            qDebug() << "Error: Failed to save image as .jpg file.";
        }
    } else {
        qDebug() << "Error: Failed to load image from QByteArray.";
    }
}


void Server::slotReadyRead()
{
    socket = (QTcpSocket *)sender();

    QString request;
    request = socket->readLine();
    qDebug() << request;
    QJsonDocument jsonRequest = QJsonDocument::fromJson(request.toUtf8());
    QJsonObject jsonObject = jsonRequest.object();
    QJsonObject jsonResponce;
    qDebug() << jsonObject["type_request"].toString();

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
        while(!tasks[name].isEmpty()){
            QString cur_task = tasks[name].pop();
            tasks_list.push_back(cur_task.mid(5, cur_task.lastIndexOf("_")-5));
        }
        jsonResponce["tasks"] = tasks_list;
    }
    else if (jsonObject["type_request"].toString() == "SEND") {
        // User send me your solution
        qDebug() << "SEND responce";
        QString path = jsonObject["team_name"].toString() + "_" +
                jsonObject["task_name"].toString()+"_"+
                jsonObject["type_answer"].toString()+"_"+
                QDateTime::currentDateTime().toString("hh_mm_ss");
        qDebug() << path;
        qDebug() << "type_answer " << jsonObject["type_answer"].toInt();
        /* todo: get file answer readall and save */
        switch (jsonObject["type_answer"].toInt()) {
        case 0:{
            // short answer
            qDebug() << this->path + "/review/" + path + ".txt";
            QFile file(this->path + "/review/" + path + ".txt");
            if(file.open(QIODevice::WriteOnly)){
                QTextStream stream(&file);
                stream << jsonObject["answer"].toString();
            }
            break;
        }
        case 1:{
            // long answer
            QFile file(this->path + "/review/" + path + ".txt");
            if(file.open(QIODevice::WriteOnly)){
                QTextStream stream(&file);
                stream << jsonObject["answer"].toString();
            }
            break;
        }
        case 2:{
            // picture answer
            qint32 messageSize = jsonObject["len"].toInt();
            QByteArray messageData;
            for (int i = 0; i < messageSize; ) {
                QString data = socket->readLine();
                i += data.size(); // BALABOLA
                qDebug() << data;
                //data.remove(data.size()-2, 2);
                messageData.append((data).toInt());
            }
            qDebug() << messageData;
            saveByteArrayAsJpg(messageData, this->path + "/review/" + path);
            break;
        }
        }

        emit newSolution(path);
    }
    else if (jsonObject["type_request"].toString() == "GET") {
        // User ask me a task
        qDebug() << "GET response";
        QDir directory = this->path;
        QStringList tasksFileNames = directory.entryList(
                    QStringList() << "task_" + jsonObject["task_name"].toString() + "_*.txt"
                                  << "task_" + jsonObject["task_name"].toString() + "_*.jpg"
                                  << "task_" + jsonObject["task_name"].toString() + "_*.jpeg"
                                  << "task_" + jsonObject["task_name"].toString() + "_*.JPG",
                    QDir::Files);
        qDebug() << tasksFileNames.size();

        QString taskFileName = this->path + "/" + tasksFileNames[0];
        qDebug() << taskFileName;


        QFile* localFile = new QFile(taskFileName);
        if (!localFile->open(QIODevice::ReadOnly)) {
                    qDebug() << "Не удалось открыть файл!";
                    return;
                }
        //qDebug() << localFile->readAll();
        qDebug() << localFile;
        qint64 totalBytes = localFile->size();
        qDebug() << totalBytes;

        QByteArray file_bytes; // Construct a QByteArray object
        QDataStream sendout(&file_bytes,QIODevice::WriteOnly);
        sendout.setVersion(QDataStream::Qt_4_8);
        sendout << localFile;

        QJsonDocument doc(jsonResponce);
        //todo: add typeConditional & typeSending
        QString strResponce(doc.toJson(QJsonDocument::Compact));
    //    socket->write(strResponce.toLocal8Bit());

       // socket->flush(); // I need to send json file (I think that client read this and he's ready to get file)
        sendout<<qint64(0)<<qint64(0);
        sendout.device()->seek(0);
        sendout << taskFileName;
        totalBytes += file_bytes.size();
        sendout<<localFile->size();
        sendout << localFile->readLine();
        qDebug() << file_bytes;
        socket->write(file_bytes);
        socket->flush();
//        socket->write("fileStart");
//        socket->flush();
//        socket->write(file_bytes, totalBytes);
//        qDebug() << file_bytes;
//        socket->flush();
//        socket->write("fileEnd");
//        socket->close();
        return;
        /*if(tasksFileNames.size()){
            // text conditional
            QString taskFileName = tasksFileNames[0];
            qDebug() << this->path + "/" + taskFileName;
            QFile file(path + "/" + taskFileName);
            jsonResponce["typeConditional"] = 0;
            jsonResponce["typeSending"] = taskFileName.mid(taskFileName.length() - 5, 1).toInt();
            if(file.open(QIODevice::ReadOnly)){
                QTextStream in(&file);
                QString task = in.readAll();
                qDebug() << task;
                jsonResponce["conditional"] = task;
            }
        }else{
            tasksFileNames = directory.entryList(
                                    QStringList() << "task_" + jsonObject["task_name"].toString() + "_*.jpg"
                                                  << "task_" + jsonObject["task_name"].toString() + "_*.jpeg"
                                                  << "task_" + jsonObject["task_name"].toString() + "_*.JPG",
                                    QDir::Files);

            QString taskFileName = tasksFileNames[0];
            QImage image;
            image.load(taskFileName);
            QByteArray image_bytes; // Construct a QByteArray object
            QBuffer buffer(&image_bytes);   // Construct a QBuffer object using the QbyteArray
            image.save(&buffer, "jpg");

            jsonResponce["typeConditional"] = 1;
            jsonResponce["typeSending"] = taskFileName.mid(
                        taskFileName.length() - 5,
                        taskFileName.length() - 5);
            QJsonDocument doc(jsonResponce);
            QString strResponce(doc.toJson(QJsonDocument::Compact));
            socket->write(strResponce.toLocal8Bit());
            socket->flush(); // I need to send json file (I think that client read this and he's ready to get file)

            socket->write("fileStart");
            socket->flush();
            socket->write(image_bytes);
            socket->flush();
            socket->write("fileEnd");
            socket->close();
            return;
        }
        */
    }

    QJsonDocument doc(jsonResponce);
    QString strResponce(doc.toJson(QJsonDocument::Compact));
    qDebug() << strResponce;

    socket->write(strResponce.toLocal8Bit());
    socket->close();
}

void Server::newPath(QString path)
{
    this->path = path;
}

void Server::newTask(QString User, QString task)
{
    this->tasks[User].push(task);
}


