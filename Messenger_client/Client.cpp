#include "Client.h"


Client::Client(const QString& strHost,
                   int            nPort,
                   QWidget*       pwgt /*=0*/
                  ) : QWidget(pwgt)
                    , m_nNextBlockSize(0)
{
    loginHeader = "LOGIN";
    sendMessageHeader = "MESSAGE";

    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(slotError(QAbstractSocket::SocketError))
           );

    m_ptxtInfo  = new QTextEdit;
    m_ptxtInput = new QLineEdit;

    connect(m_ptxtInput, SIGNAL(returnPressed()),
            this,        SLOT(slotSendToServer())
           );
    m_ptxtInfo->setReadOnly(true);

    m_header = new QLabel("<H1>Client</H1>");

    QPushButton* pcmd = new QPushButton("&Send");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));

    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(m_header);
    pvbxLayout->addWidget(m_ptxtInfo);
    pvbxLayout->addWidget(m_ptxtInput);
    pvbxLayout->addWidget(pcmd);
    setLayout(pvbxLayout);
}


void Client::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_2);

    QString message;

    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime   time;
        QString str;
        in >> time >> str;

        message += str;

        m_ptxtInfo->append(time.toString() + " " + str);
        m_nNextBlockSize = 0;
    }
    m_ptxtInfo->append("msg: \"" + message + "\"");
    processIncomingData(message);
}




void Client::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    m_ptxtInfo->append(strError);
}


void Client::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << QTime::currentTime() << m_ptxtInput->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
    m_ptxtInput->setText("");
}


void Client::slotConnected()
{
    m_ptxtInfo->append("Received the connected() signal");
}


void Client::processIncomingData(QString &msg){
     std::string_view stdMsg = msg.toStdString();

     if(msg.startsWith(loginHeader)){
         stdMsg.remove_prefix(loginHeader.length() + 1);
         processLogin(stdMsg.data());
     }
     else if(msg.startsWith(sendMessageHeader)){
         stdMsg.remove_prefix(sendMessageHeader.length() + 1);
         processMessage(stdMsg.data());
     }
}

void Client::processLogin(std::string msg){
    if(msg[0] == '1'){
        m_login = msg.substr(2);
        m_ptxtInfo->append("Login successed! Hi, " + QString::fromStdString(m_login) + "!");

        m_header->setText(m_header->text() + "(" + QString::fromStdString(m_login) + ")");
    }
    else{
        m_ptxtInfo->append("Failed login");
    }
}

void Client::processMessage(std::string msg){
    std::string sender_login = msg.substr(0, msg.find(' '));
    std::string message_text = msg.substr(msg.find(' ') + 1);

    m_ptxtInfo->append(QString::fromStdString(sender_login) + " sent to you: \"" + QString::fromStdString(message_text) + "\"");
}

