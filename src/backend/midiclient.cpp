#include "midiclient.h"

MidiClient::MidiClient(QObject *parent)
    : QObject(parent), m_bankNumber(0)  {
    jackClient = new JackClient;
    connect(jackClient, &JackClient::midiMessageReceived, this, &MidiClient::handleMidiMessage);
    getIOPorts();
    QTimer *connectionCheckTimer = new QTimer(this);
    connect(connectionCheckTimer, &QTimer::timeout, this, &MidiClient::checkOutputPortConnection);
    connectionCheckTimer->start(1000); // Check every second

}
void MidiClient::handleMidiMessage(const libremidi::message& message)
{
    // Handle the received MIDI message here
    // e.g., update UI, process the message, etc.
    qDebug()<< message;
    int bankNumber = 0;
    if (isGenosRegistrationBankChange(message, bankNumber)) {
        qDebug() << "Received registration bank change for bank" << bankNumber;
        setBankNumber(bankNumber);
    }

    if(itsVolumeCC(message))
    {
        int statusByte = message[0];
        int channel = statusByte & 0x0F; // Mask the lowest 4 bits
        int midivolume=message[2];
        int volume=( midivolume * 100) / 127;
        if(channel==15){
            qDebug()<<"SEND MIDI VOLUME "<<volume;
        }
    }
    if(isNextPagesCC(message))
    {
        int statusByte = message[0];
        int channel = statusByte & 0x0F; // Mask the lowest 4 bits
        if(channel==15){
            qDebug()<<"Next Pages MIDI ";
            emit goToNextPage();
        }
    }
    if(!itsVolumeCC(message)){
        // jackClient->midiout_raw->send_message(message);
    }
    if (itsNote(message)) {

        // Extract the note and velocity from the message
        int note = message[1];
        int velocity = message[2];
        // Check if the message is a note-off
        int statusByte = message[0];
        int channel = statusByte & 0x0F; // Mask the lowest 4 bits
        int messageType = statusByte & 0xF0; // Mask the highest 4 bits
        if (messageType == 0x80) {
            // It's a note-off message, send note-off with velocity 0
            velocity = 0;
        }

        if(channel == 0){
            emit channelActivated(0);

        }
        else if(channel == 1){
        }
        else if(channel == 2){

            emit channelActivated(2);
        }
    }
}
bool MidiClient::itsNote(const libremidi::message& message)
{
    if (!message.empty()) {
        int statusByte = message[0];
        int messageType = statusByte & 0xF0; // Mask the highest 4 bits

        // Check if the message is a Note On or Note Off
        return (messageType == 0x80 || messageType == 0x90);
    }

    return false;
}
void MidiClient::sendControlChange(int channel, int control, int value)
{
    jackClient->sendMidiMessage(0, libremidi::channel_events::control_change(channel, control, value));
}


void MidiClient::sendRawMessage(const libremidi::message& message)
{
    jackClient->sendMidiMessage(0, message);
}


void MidiClient::sendAllNotesOff()
{
    // Send the "All Notes Off" message for the specified channel
    for(int i=1;i<=16;i++){
        jackClient->sendMidiMessage(0, libremidi::channel_events::control_change(i, 123, 0));    }
}
void MidiClient::sendNotesOff(int channel)
{
    // Send the "Notes Off" message for the specified channel
    jackClient->sendMidiMessage(0, libremidi::channel_events::control_change(channel+1, 123, 0));
}
void MidiClient::sendMsbLsbPc(int channel, int msb, int lsb, int pc)
{
    // Ensure the channel is in the valid range (1-16 in MIDI, but 0-15 in some APIs)
    if (channel < 0 || (channel > 15))
        return;

    // Ensure MSB, LSB, and PC values are in the valid MIDI range (0-127)
    msb = qBound(0, msb, 127);
    lsb = qBound(0, lsb, 127);
    pc = qBound(0, pc, 127);


    // Send the MSB, LSB, and PC messages
    jackClient->sendMidiMessage(0, libremidi::channel_events::control_change(channel+1, 0x00, msb));  // MSB (0x00)
    jackClient->sendMidiMessage(0, libremidi::channel_events::control_change(channel+1, 0x20, lsb));  // LSB (0x20)
    jackClient->sendMidiMessage(0, libremidi::channel_events::program_change(channel+1, pc));         // PC
    qDebug() << "Sent MSB:" << msb << "LSB:" << lsb << "PC:" << pc<< "on channel:" << channel;
}

void MidiClient::getIOPorts(){

    if (jackClient->observer.has_value()) { // Check if optional has a value
        m_inputPorts->clear();
        libremidi::observer& obs = jackClient->observer.value(); // Dereference optional to get the underlying libremidi::observer object
        for(const libremidi::input_port& port : obs.get_input_ports()) {
            //qDebug()<< port.port_name;
            //   jackClient->midiin->open_port(port,"In");
            m_inputPorts->addPort(QString::fromStdString(port.port_name), QVariant::fromValue(port));
        }
    }
    if (jackClient->observer.has_value()) { // Check if optional has a value
        m_outputPorts->clear();
        libremidi::observer& obs = jackClient->observer.value(); // Dereference optional to get the underlying libremidi::observer object
        for(const libremidi::output_port& port : obs.get_output_ports()) {
            //  qDebug()<< port.port_name;
            m_outputPorts->addPort(QString::fromStdString(port.port_name), QVariant::fromValue(port));
            //   jackClient->midiout->open_port(port,"Out");

        }
    }
}

void MidiClient::makeConnection(QVariant inputPorts,QVariant outputPorts){

    if (inputPorts.isValid()) {
        jackClient->midiin->close_port();
        libremidi::input_port selectedInputPort = qvariant_cast<libremidi::input_port>(inputPorts);
        jackClient->midiin->open_port(selectedInputPort,"In");
    }
    else {
        // Handle case when no port is selected
        qDebug() << "No input port selected.";
    }
    // Use the selected ports as needed
    if (outputPorts.isValid()) {
        jackClient->midiout->close_port();
        libremidi::output_port selectedOutputPort = qvariant_cast<libremidi::output_port>(outputPorts);
        jackClient->midiout->open_port(selectedOutputPort,"Out");
    }
    else {
        // Handle case when no port is selected
        qDebug() << "No output port selected.";
    }

    emit outputPortConnectionChanged();
}

void MidiClient::makeDisconnect(){
    jackClient->midiin->close_port();
    jackClient->midiout->close_port();

    // Handle case when no port is selected
    emit outputPortConnectionChanged();
    qDebug() << "Disconnected";

}
void MidiClient::checkOutputPortConnection(){
    bool currentStatus = isOutputPortConnected();
    if (currentStatus != m_lastOutputPortStatus) {
        m_lastOutputPortStatus = currentStatus;
        emit outputPortConnectionChanged();
    }
}

void MidiClient::setCc(bool cc) {
    if (m_cc != cc) {
        m_cc = cc;
        emit ccChanged();
    }
}
void MidiClient::setPc(bool pc) {
    if (m_pc != pc) {
        m_pc = pc;
        emit pcChanged();
    }
}
bool MidiClient::cc() const {
    return m_cc;
}
bool MidiClient::pc() const {
    return m_pc;
}


bool MidiClient::itsVolumeCC(const libremidi::message& message)
{
    if (!message.empty()) {
        int statusByte = message[0];
        int messageType = statusByte & 0xF0; // Mask the highest 4 bits

        // Check if the message is a Control Change (CC) message
        if (messageType == 0xB0 && message.size() > 1) {
            int controlNumber = message[1]; // The second byte is the control number

            // Check if the control number is 7 (which is the standard for volume)
            return (controlNumber == 7);
        }
    }

    return false;
}
bool MidiClient::isNextPagesCC(const libremidi::message& message)
{
    if (!message.empty()) {
        int statusByte = message[0];
        int messageType = statusByte & 0xF0; // Mask the highest 4 bits

        // Check if the message is a Control Change (CC) message
        if (messageType == 0xB0 && message.size() > 1) {
            int controlNumber = message[1]; // The second byte is the control number

            // Check if the control number is 7 (which is the standard for volume)
            return (controlNumber == 64);
        }
    }

    return false;
}
QString MidiClient::noteNumberToName(int noteNumber) const
{
    if (noteNumber < 0 || noteNumber > 127) {
        return "Invalid"; // Handle out of range notes
    }

    const QStringList noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = noteNumber / 12 - 1;
    QString note = noteNames[noteNumber % 12];

    return QString("%1%2").arg(note).arg(octave);
}
void MidiClient::sendRegistrationChange(int bankNumber)
{


    // Ensure bankNumber is within the range 1-10
    if (bankNumber < 1 || bankNumber > 10) {
        qWarning() << "Invalid bank number. Please use a value between 1 and 10.";
        return;
    }

    // Create a libremidi::message directly using initializer list
    libremidi::message message{
        240, 67, 115, 1, 82, 37, 17, 0, 2, 0, static_cast<unsigned char>(bankNumber - 1), 247
    };

    // Send the SysEx message
    // jackClient->send_MidiMessage(message);
}

bool MidiClient::isGenosRegistrationBankChange(const libremidi::message& message, int& bankNumber)
{
    // Check if the message is a SysEx message
    if (message[0] != 0xF0 || message.back() != 0xF7) {
        return false; // Not a SysEx message
    }

    // Check if the message is a Yamaha Genos registration bank change
    // SysEx message structure: 240, 67, 115, 1, 82, 37, 17, 0, 2, 0, <bank>, 247
    if (message.size() == 12 &&
            message[1] == 67 &&  // Yamaha Manufacturer ID
            message[2] == 115 && // Extended ID
            message[3] == 1 &&   // Device ID (can vary)
            message[4] == 82 &&  // Command ID
            message[5] == 37 &&  // Sub-command
            message[6] == 17 &&  // Indicates registration memory change
            message[7] == 0 &&
            message[8] == 2 &&
            message[9] == 0)
    {
        // Extract the bank number from the second-to-last byte
        bankNumber = message[10] + 1; // Adjust to 1-based index for banks 1-10
        return true;
    }

    return false; // Not a registration bank change message
}
int MidiClient::bankNumber() const
{
    return m_bankNumber;
}

void MidiClient::setBankNumber(int newBankNumber)
{
    if (m_bankNumber != newBankNumber) {
        m_bankNumber = newBankNumber;
        emit bankNumberChanged(m_bankNumber);  // Emit signal when the bankNumber changes
    }
}
