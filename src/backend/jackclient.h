#ifndef JACKCLIENT_H
#define JACKCLIENT_H

#include <QObject>
#include <libremidi/configurations.hpp>
#include <libremidi/detail/memory.hpp>
#include <libremidi/libremidi.hpp>
#include <libremidi/message.hpp>
#include <jack/jack.h>


class JackClient : public QObject
{
    Q_OBJECT
public:
    explicit JackClient(QObject *parent = nullptr);
    static int jack_callback(jack_nframes_t cnt, void* ctx);
    std::optional<libremidi::observer> observer;

    std::unique_ptr<libremidi::midi_in> midiin;
    std::unique_ptr<libremidi::midi_out> midiout;

signals:
    void midiMessageReceived(const libremidi::message& message);
public slots:
    void sendMidiMessage(int port, const libremidi::message& message);
    void send_MidiMessage(const libremidi::message message);
private:
    libremidi::unique_handle<jack_client_t, jack_client_close> handle;

    libremidi::jack_callback midiin_callback;
    libremidi::jack_callback midiout_callback;



};

#endif // JACKCLIENT_H
