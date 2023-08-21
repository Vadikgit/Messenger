#include "Server.h"


Server::Server(int nPort, QWidget* pwgt /*=0*/) : QWidget(pwgt)
                                                    , m_nNextBlockSize(0)
{
    loginHeader = "LOGIN";
    sendMessageHeader = "MESSAGE";

    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0,
                              "Server Error",
                              "Unable to start the server:"
                              + m_ptcpServer->errorString()
                             );
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer, SIGNAL(newConnection()),
            this,         SLOT(slotNewConnection())
           );

    m_ptxt = new QTextEdit;
    m_ptxt->setReadOnly(true);

    // get logins and passwords from file
    getLoginsPasswords();

    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Server</H1>"));
    pvbxLayout->addWidget(m_ptxt);
    setLayout(pvbxLayout);
}



void Server::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    QWidget::connect(pClientSocket, SIGNAL(disconnected()),
            pClientSocket, SLOT(deleteLater())
           );
    connect(pClientSocket, SIGNAL(readyRead()),
            this,          SLOT(slotReadClient())
           );

    sendToClient(pClientSocket, "Server Response: Connected!");
}


void Server::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_4_2);

    QString message;

    for (;;) {
        if (!m_nNextBlockSize) {
            if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (pClientSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime   time;
        QString str;
        in >> time >> str;

        message += str;

        QString strMessage =
            time.toString() + " " + "Client " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) +
                + " has sended - " + str;
        m_ptxt->append(strMessage);

        m_nNextBlockSize = 0;

        //sendToClient(pClientSocket, "Server Response: Received " + QString::number(str.length()) + " bytes\n");  - bug
    }

    processIncomingData(pClientSocket, message);
}



void Server::sendToClient(QTcpSocket* pSocket, const QString& str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void Server::getLoginsPasswords(){
    std::ifstream loginsPasswordsFile("login_password.txt");

    if (!loginsPasswordsFile)
    {
        m_ptxt->append("login_password.txt could not be opened for reading!\n");
        return;
    }

    m_ptxt->append("Founded login credentials:\n");

    while (loginsPasswordsFile)
    {
      std::string login, password;
      loginsPasswordsFile >> login >> password;

      login_password_table.emplace(login, password);

      QString strMessage = "\t" + QString::fromStdString(login) + " " + QString::fromStdString(password) + "\n";
      m_ptxt->append(strMessage);
    }

}

void Server::processIncomingData(QTcpSocket* pSocket, QString & msg){
    std::string_view stdMsg = msg.toStdString();

    if(msg.startsWith(loginHeader)){
        stdMsg.remove_prefix(loginHeader.length() + 1);
        processLogin(pSocket, stdMsg.data());
    }
    else if(msg.startsWith(sendMessageHeader)){
        stdMsg.remove_prefix(sendMessageHeader.length() + 1);
        processMessage(pSocket, stdMsg.data());
    }
}

void Server::processLogin(QTcpSocket* pSocket, std::string msg){

    std::string login, password;

    int passwordStart = msg.find(' ');

    login = msg.substr(0, passwordStart);
    password = msg.substr(passwordStart + 1);

    QString strMessage = "Trying of login:\n" + QString::fromStdString(login.data()) + " " + QString::fromStdString(password.data());
    m_ptxt->append(strMessage);

    auto it = login_password_table.find(login);
    if(it != login_password_table.end() && it->second == password){
        m_ptxt->append("\nLoging successed!\n");

        sendToClient(pSocket, loginHeader + " 1 " + QString::fromStdString(login));

        login_to_socket[login] = pSocket;
        std::string host_and_port = pSocket->peerAddress().toString().toStdString() + ":" + QString::number(pSocket->peerPort()).toStdString();
        host_and_port_to_login[host_and_port] = login;
    }
    else{
        m_ptxt->append("\nLoging failed!\n");

        sendToClient(pSocket, loginHeader + " 0");
    }
}


void Server::processMessage(QTcpSocket *pSocket, std::string msg){

    std::string sender_login = host_and_port_to_login[pSocket->peerAddress().toString().toStdString() + ":" + QString::number(pSocket->peerPort()).toStdString()];
    std::string receiver_login = msg.substr(0, msg.find(' '));
    std::string message_text = msg.substr(msg.find(' ') + 1);

    QString strMessage = "Sending message:\n From: " + QString::fromStdString(sender_login) + "\nTo: " + QString::fromStdString(receiver_login) + "\n Content: " + QString::fromStdString(message_text);
    m_ptxt->append(strMessage);

    sendToClient(login_to_socket[receiver_login], "MESSAGE " + QString::fromStdString(sender_login) + " " + QString::fromStdString(message_text));
}
