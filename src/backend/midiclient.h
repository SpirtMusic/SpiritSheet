#ifndef MIDICLIENT_H
#define MIDICLIENT_H

#include <QObject>
#include <QTimer>
#include <backend/jackclient.h>
#include <backend/midiutils.h>
#include <backend/midiportmodel.h>

class MidiClient  : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MidiPortModel* inputPorts READ inputPorts CONSTANT)
    Q_PROPERTY(MidiPortModel* outputPorts READ outputPorts CONSTANT)
    Q_PROPERTY(bool isOutputPortConnected READ isOutputPortConnected NOTIFY outputPortConnectionChanged)
    Q_PROPERTY(bool cc READ cc WRITE setCc NOTIFY ccChanged)
    Q_PROPERTY(bool pc READ pc WRITE setPc NOTIFY pcChanged)

public:
    explicit MidiClient(QObject *parent = nullptr);
    MidiPortModel* inputPorts() const { return m_inputPorts; }
    MidiPortModel* outputPorts() const { return m_outputPorts; }
    bool isOutputPortConnected() const {
        return jackClient && jackClient->midiout && jackClient->midiout->is_port_connected();
    }
    bool cc() const;
    bool pc() const;
    int bankNumber() const;

signals:
    void outputPortConnectionChanged();
    void channelActivated(int channel);
    void ccChanged();
    void pcChanged();
    void bankNumberChanged(int newBankNumber);
    void goToNextPage();
public slots:
    Q_INVOKABLE void sendControlChange(int channel, int control, int value);
    Q_INVOKABLE void sendRawMessage(const libremidi::message& message);
    Q_INVOKABLE void sendAllNotesOff();
    Q_INVOKABLE void sendMsbLsbPc(int channel, int msb, int lsb, int pc);
    Q_INVOKABLE void sendNotesOff(int channel);
    Q_INVOKABLE void getIOPorts();
    Q_INVOKABLE void makeConnection(QVariant inputPorts, QVariant outputPorts);
    Q_INVOKABLE void makeDisconnect();
    void checkOutputPortConnection();
    void setCc(bool cc);
    void setPc(bool pc);
    void setBankNumber(int newBankNumber);
    Q_INVOKABLE void sendRegistrationChange(int bankNumber);

private slots:
    void handleMidiMessage(const libremidi::message& message);

private:
    JackClient *jackClient;
    bool itsNote(const libremidi::message& message);
    bool itsVolumeCC(const libremidi::message& message);
     bool isNextPagesCC(const libremidi::message& message);
    bool isGenosRegistrationBankChange(const libremidi::message& message, int& bankNumber);
    QString noteNumberToName(int noteNumber) const;

    MidiPortModel *m_inputPorts;
    MidiPortModel *m_outputPorts;
    bool m_lastOutputPortStatus = false;
    bool m_cc;
    bool m_pc;
    int m_currentChannel = 0;
    int m_bankNumber;
};

#endif // MIDICLIENT_H
