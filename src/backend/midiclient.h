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



    Q_PROPERTY(int midiChannel READ midiChannel WRITE setMidiChannel NOTIFY midiChannelChanged)
    Q_PROPERTY(int nextPageControl READ nextPageControl WRITE setNextPageControl NOTIFY nextPageControlChanged)
    Q_PROPERTY(int prevPageControl READ prevPageControl WRITE setPrevPageControl NOTIFY prevPageControlChanged)
    Q_PROPERTY(QString currentMidiDevice READ currentMidiDevice WRITE setCurrentMidiDevice NOTIFY currentMidiDeviceChanged)



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



    int midiChannel() const { return m_midiChannel; }
    int nextPageControl() const { return m_nextPageControl; }
    int prevPageControl() const { return m_prevPageControl; }
    QString currentMidiDevice() const { return m_currentMidiDevice; }

    // Add setters
    void setMidiChannel(int channel);
    void setNextPageControl(int control);
    void setPrevPageControl(int control);
    void setCurrentMidiDevice(const QString &device);



signals:
    void outputPortConnectionChanged();
    void channelActivated(int channel);
    void ccChanged();
    void pcChanged();
    void bankNumberChanged(int newBankNumber);





    void midiChannelChanged(int channel);
    void nextPageControlChanged(int control);
    void prevPageControlChanged(int control);
    void currentMidiDeviceChanged(QString device);

    void goToNextPage();
    void goToPreviousPage();

    void midiMessageReceived(int channel, int control, int value);

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
        bool isPrevPagesCC(const libremidi::message& message);
    bool isGenosRegistrationBankChange(const libremidi::message& message, int& bankNumber);
    QString noteNumberToName(int noteNumber) const;

    MidiPortModel *m_inputPorts;
    MidiPortModel *m_outputPorts;
    bool m_lastOutputPortStatus = false;
    bool m_cc;
    bool m_pc;
    int m_currentChannel = 0;
    int m_bankNumber;



    int m_midiChannel = 0;
    int m_nextPageControl = 64;  // Default to sustain pedal
    int m_prevPageControl = 67;  // Default to soft pedal
    QString m_currentMidiDevice;


};

#endif // MIDICLIENT_H
