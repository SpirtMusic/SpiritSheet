#include "jackclient.h"
#include <QDebug>
JackClient::JackClient(QObject *parent)
    : QObject{parent}, midiin(nullptr),midiout(nullptr)

{
    auto callback = [&](int port, const libremidi::message& message) {
        // qDebug() << message;
        emit midiMessageReceived(message);
        //  midiout[port].send_message(message);
    };

    // Create a JACK client which will be shared across objects
    jack_status_t status{};
    handle.reset(jack_client_open("SpiritSheet", JackNoStartServer, &status));

    if (!handle)
        throw std::runtime_error("Could not start JACK client");
    // Create an observer configuration
    // libremidi::observer_configuration conf{
    //     .input_added = [&](const libremidi::input_port& id) {
    //         qDebug() << "Input connected: " << id.port_name;
    //     },
    //     .input_removed = [&](const libremidi::input_port& id) {
    //         qDebug() << "Input removed: " << id.port_name;
    //     },
    //     .output_added = [&](const libremidi::output_port& id) {
    //         qDebug() << "Output connected: " << id.port_name;
    //     },
    //     .output_removed = [&](const libremidi::output_port& id) {
    //         qDebug() << "Output removed: " << id.port_name;
    //     }
    // };

    // // Create an observer using the configuration
    // observer = libremidi::observer{conf, libremidi::jack_observer_configuration{.context = handle.get()}};
    jack_set_process_callback(handle.get(), jack_callback, this);
    jack_activate(handle.get());


    // Create our configuration
    auto api_input_config = libremidi::jack_input_configuration{
        .context = handle.get(),
        .set_process_func = [this](libremidi::jack_callback cb) {
            midiin_callback = std::move(cb);
        }
    };

    auto api_output_config = libremidi::jack_output_configuration{
        .context = handle.get(),
        .set_process_func = [this](libremidi::jack_callback cb) {
            midiout_callback = std::move(cb);
        }
    };


    midiin = std::make_unique<libremidi::midi_in>(
        libremidi::input_configuration{
            .on_message = [=](const libremidi::message& msg) { callback(0, msg); },
            .ignore_sysex = false
        },
        api_input_config
        );
    // midiin->open_virtual_port("Input: 1");

    midiout = std::make_unique<libremidi::midi_out>(
        libremidi::output_configuration{},
        api_output_config
        );
    //  midiout->open_virtual_port("Output: 1");
}

int JackClient::jack_callback(jack_nframes_t cnt, void *ctx)
{
    auto& self = *(JackClient*)ctx;

    // Process the midi input
    self.midiin_callback.callback(cnt);
    // Process the midi output
    self.midiout_callback.callback(cnt);
    return 0;
}
void JackClient::sendMidiMessage(int port, const libremidi::message& message)
{
    midiout->send_message(message);
}
