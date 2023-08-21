#ifndef _Server_h_
#define _Server_h_

#include <QWidget>
#include <QTextEdit>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>
#include <unordered_map>
#include <string>
#include <fstream>
#include <QDir>
#include <iostream>
#include <QString>
#include <QStringView>
#include <string_view>

class Server : public QWidget {
Q_OBJECT
private:
    std::unordered_map<std::string, std::string> login_password_table;
    std::unordered_map<std::string, QTcpSocket *> login_to_socket;
    std::unordered_map<std::string, std::string> host_and_port_to_login;

    QTcpServer* m_ptcpServer;
    QTextEdit*  m_ptxt;
    quint16     m_nNextBlockSize;

private:
    void getLoginsPasswords();
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    void processIncomingData(QTcpSocket* pSocket, QString & msg);
    void processLogin(QTcpSocket* pSocket, std::string msg);
    void processMessage(QTcpSocket* pSocket, std::string msg);

    QString loginHeader;
    QString sendMessageHeader;

public:
    Server(int nPort, QWidget* pwgt = 0);

public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();
};

#endif  //_Server_h_
