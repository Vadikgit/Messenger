#ifndef _Client_h_
#define _Client_h_

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>
#include <QString>
#include <string>


class Client : public QWidget {
Q_OBJECT
private:
QTcpSocket* m_pTcpSocket;
    QTextEdit*  m_ptxtInfo;
    QLineEdit*  m_ptxtInput;
    quint16     m_nNextBlockSize;
    QLabel*     m_header;

    QString loginHeader;
    QString sendMessageHeader;


    std::string m_login;

    void processIncomingData(QString & msg);
    void processLogin(std::string msg);
    void processMessage(std::string msg);

public:
    Client(const QString& strHost, int nPort, QWidget* pwgt = 0) ;

private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(                            );
    void slotConnected   (                            );
};
#endif  //_Client_h_
