
#include "PortListener.h"
#include <QtDebug>

PortListener::PortListener()
{
    port = NULL;
}

void PortListener::init(const QString & portName, BaudRateType baud)
{
    if(port != NULL) {
         // don't reinitialize port
        if(port->portName() == portName)
            return;
        disconnect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead())); // never disconnect
        delete port;
    }
    this->port = new QextSerialPort(portName, QextSerialPort::EventDriven);
    port->setBaudRate(baud);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void PortListener::setDtr(bool enable)
{
    this->port->setDtr(enable);
}

bool PortListener::open()
{
    if(!textEditor) // no text editor, no open
        return false;

    if(port == NULL)
        return false;

    return port->open(QIODevice::ReadWrite);
}

void PortListener::close()
{
    if(port == NULL)
        return;

    if(port->isOpen())
        port->close();
}

void PortListener::setTerminalWindow(QPlainTextEdit *editor)
{
    textEditor = editor;
}

void PortListener::send(QByteArray &data)
{
    port->write(data.constData(),1);
}

void PortListener::onReadyRead()
{
    const int blen = 1024;
    char buff[blen+1];
    int len = port->bytesAvailable();
    if(len > blen)
        len = blen;
    int ret = port->read(buff, len);

    if(ret > -1)
    {
        buff[ret] = '\0';
        QString sbuff(buff);
        if(sbuff.indexOf('\b') < 0) // if no backspace, just append buffer
        {
            textEditor->setPlainText(textEditor->toPlainText() + buff);
            textEditor->moveCursor(QTextCursor::End);
        }
        else // if backspaces, do it the slow way.
        {
            for(int n = 0; n < ret; n++)
            {
                QString text = textEditor->toPlainText();
                int tlen = text.length();
                if(buff[n] == '\b')
                    textEditor->setPlainText(text.mid(0,tlen-1));
                else
                    textEditor->setPlainText(text + buff[n]);
            }
            textEditor->moveCursor(QTextCursor::End);
        }
        textEditor->repaint(); // yay!
    }
}

void PortListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}
