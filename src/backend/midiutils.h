#ifndef MIDIUTILS_H
#define MIDIUTILS_H
#include <QByteArray>
#include <QDebug>
#include <libremidi/libremidi.hpp>
#include <libremidi/cmidi2.hpp>
#if LIBREMIDI_USE_NI_MIDI2
#include <midi/universal_packet.h>
#endif
inline QDebug operator<<(QDebug s, const libremidi::message& message)
{
    QString bytes;
    for (auto byte : message) {
        bytes += QString::number(static_cast<int>(byte)) + " ";
    }

    s.noquote() << "[ " << bytes << "]; stamp = " << message.timestamp;

    // Extract the channel from the status byte
    if (!message.empty()) {
        int statusByte = message[0];
        int channel = statusByte & 0x0F; // Mask the lowest 4 bits
        s << "; channel = " << channel + 1; // Channels are 1-based

        // Determine the message type
        int messageType = statusByte & 0xF0; // Mask the highest 4 bits
        switch (messageType) {
        case 0x80: // Note Off
            s << "; Note Off";
            // Perform your action for Note Off messages
            break;
        case 0x90: // Note On
            s << "; Note On";
            // Perform your action for Note On messages
            break;
        case 0xB0: // Control Change
            s << "; Control Change";
            // Perform your action for Control Change messages
            break;
        case 0xC0: // Program Change
            s << "; Program Change";
            // Perform your action for Program Change messages
            break;
        // Add more cases for other message types as needed
        default:
            break;
        }
    }

    return s;
}

inline QDebug operator<<(QDebug s, const libremidi::ump& message)
{
    const cmidi2_ump* b = message;
    int bytes = cmidi2_ump_get_num_bytes(message.data[0]);
    int group = cmidi2_ump_get_group(b);
    int status = cmidi2_ump_get_status_code(b);
    int channel = cmidi2_ump_get_channel(b);
    s.noquote() << "[ " << bytes << " | " << group;

    switch (static_cast<libremidi::message_type>(status)) {
    case libremidi::message_type::NOTE_ON:
        s << " | note on: " << channel << static_cast<int>(cmidi2_ump_get_midi2_note_note(b)) << " | "
          << cmidi2_ump_get_midi2_note_velocity(b);
        break;
    case libremidi::message_type::NOTE_OFF:
        s << " | note off: " << channel << static_cast<int>(cmidi2_ump_get_midi2_note_note(b)) << " | "
          << cmidi2_ump_get_midi2_note_velocity(b);
        break;
    case libremidi::message_type::CONTROL_CHANGE:
        s << " | cc: " << channel << static_cast<int>(cmidi2_ump_get_midi2_cc_index(b)) << " | "
          << cmidi2_ump_get_midi2_cc_data(b);
        break;
    default:
        break;
    }
    s << " ]";
#if LIBREMIDI_USE_NI_MIDI2
    s << " :: " << midi::universal_packet(message);
#endif
    return s;
}

inline QDebug operator<<(QDebug s, const libremidi::port_information& rhs)
{
    s.noquote() << "[ client: " << rhs.client << ", port: " << rhs.port;
    if (!rhs.manufacturer.empty())
        s << ", manufacturer: " << QString::fromStdString(rhs.manufacturer);
    if (!rhs.device_name.empty())
        s << ", device: " << QString::fromStdString(rhs.device_name);
    if (!rhs.port_name.empty())
        s << ", portname: " << QString::fromStdString(rhs.port_name);
    if (!rhs.display_name.empty())
        s << ", display: " << QString::fromStdString(rhs.display_name);
    return s << "]";
}
#endif // MIDIUTILS_H
