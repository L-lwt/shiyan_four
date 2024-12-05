#include "serverworker.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
QString ServerWorker::userName()
{
    return m_userName;
}

void ServerWorker::setUserName(QString user)
{
    m_userName=user;
}

ServerWorker::ServerWorker(QObject *parent)
    : QObject{parent}
{
    m_serverSocket=new QTcpSocket(this);
    connect(m_serverSocket, &QTcpSocket::readyRead, this, &ServerWorker::onReadyRead);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

void ServerWorker::onReadyRead()
{
    // 成员变量
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);

    // 设置数据流版本
    socketStream.setVersion(QDataStream::Qt_5_12);

    // 无限循环读取数据
    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;

        if (socketStream.commitTransaction()) {
            // emit logMessage(QString::fromUtf8(jsonData));
            // sendMessage("I received message");

            QJsonParseError parseError;

            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()) { // and is a JSON object
                    emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));
                    emit jsonReceived(this, jsonDoc.object()); // parse the JSON
                }
            }

        } else {
            break;
        }
    }

    // 以下是辅助函数，未在原始代码中提供，但可能需要实现
    void sendMessage(const QString& message);

}

void ServerWorker::sendMessage(const QString &text, const QString &type)
{
    if (m_serverSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if (!text.isEmpty()) {
        // create a QDataStream operating on the socket
        QDataStream serverStream(m_serverSocket);
        serverStream.setVersion(QDataStream::Qt_5_12);

        // Create the JSON we want to send
        QJsonObject message;
        message["type"] = type;
        message["text"] = text;

        // send the JSON using QDataStream
        serverStream << QJsonDocument(message).toJson();
    }

}

void ServerWorker::sendJson(const QJsonObject &json)
{
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit logMessage(QLatin1String("Sending to ") + userName() + QLatin1String(" - ") +
                    QString::fromUtf8(jsonData));

    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_7);
    socketStream << jsonData;

}
