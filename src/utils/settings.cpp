#include "settings.h"

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(QSettings::IniFormat, QSettings::UserScope, "SpiritMusic", "SpiritSheet")
{
    QFileInfo settingsFile(m_settings.fileName());
     QDir().mkpath(settingsFile.absolutePath());

     // Initialize with default values if file doesn't exist
     if (!settingsFile.exists()) {
         resetToDefaults();
     } else {
         load();
     }
}

Settings::~Settings()
{
}

// General getters
bool Settings::autoOpenLast() const { return m_autoOpenLast; }
int Settings::defaultZoom() const { return m_defaultZoom; }
int Settings::defaultViewMode() const { return m_defaultViewMode; }

// MIDI getters
int Settings::midiChannel() const { return m_midiChannel; }
int Settings::nextPageControl() const { return m_nextPageControl; }
int Settings::prevPageControl() const { return m_prevPageControl; }
QString Settings::midiDevice() const { return m_midiDevice; }

// General setters
void Settings::setAutoOpenLast(bool value)
{
    if (m_autoOpenLast != value) {
        m_autoOpenLast = value;
        m_settings.setValue("General/AutoOpenLast", value);
        m_settings.sync();
        emit autoOpenLastChanged();
    }
}

void Settings::setDefaultZoom(int value)
{
    if (m_defaultZoom != value) {
        m_defaultZoom = value;
        m_settings.setValue("General/DefaultZoom", value);
        m_settings.sync();
        emit defaultZoomChanged();
    }
}

void Settings::setDefaultViewMode(int value)
{
    if (m_defaultViewMode != value) {
        m_defaultViewMode = value;
        m_settings.setValue("General/DefaultViewMode", value);
        m_settings.sync();
        emit defaultViewModeChanged();
    }
}

// MIDI setters
void Settings::setMidiChannel(int value)
{
    if (m_midiChannel != value) {
        m_midiChannel = value;
        m_settings.setValue("MIDI/Channel", value);
        m_settings.sync();
        emit midiChannelChanged();
    }
}

void Settings::setNextPageControl(int value)
{
    if (m_nextPageControl != value) {
        m_nextPageControl = value;
        m_settings.setValue("MIDI/NextPageControl", value);
        m_settings.sync();
        emit nextPageControlChanged();
    }
}

void Settings::setPrevPageControl(int value)
{
    if (m_prevPageControl != value) {
        m_prevPageControl = value;
        m_settings.setValue("MIDI/PrevPageControl", value);
        m_settings.sync();
        emit prevPageControlChanged();
    }
}

void Settings::setMidiDevice(const QString &device)
{
    if (m_midiDevice != device) {
        m_midiDevice = device;
        m_settings.setValue("MIDI/Device", device);
        m_settings.sync();
        emit midiDeviceChanged();
    }
}

// void Settings::save()
// {
//     // General settings
//     m_settings.setValue("General/AutoOpenLast", m_autoOpenLast);
//     m_settings.setValue("General/DefaultZoom", m_defaultZoom);
//     m_settings.setValue("General/DefaultViewMode", m_defaultViewMode);

//     // MIDI settings
//     m_settings.setValue("MIDI/Channel", m_midiChannel);
//     m_settings.setValue("MIDI/NextPageControl", m_nextPageControl);
//     m_settings.setValue("MIDI/PrevPageControl", m_prevPageControl);
//     m_settings.setValue("MIDI/Device", m_midiDevice);

//     m_settings.sync();
// }

void Settings::load()
{
    // General settings
    m_autoOpenLast = m_settings.value("General/AutoOpenLast", DEFAULT_AUTO_OPEN_LAST).toBool();
    m_defaultZoom = m_settings.value("General/DefaultZoom", DEFAULT_ZOOM).toInt();
    m_defaultViewMode = m_settings.value("General/DefaultViewMode", DEFAULT_VIEW_MODE).toInt();

    // MIDI settings
    m_midiChannel = m_settings.value("MIDI/Channel", DEFAULT_MIDI_CHANNEL).toInt();
    m_nextPageControl = m_settings.value("MIDI/NextPageControl", DEFAULT_NEXT_PAGE_CONTROL).toInt();
    m_prevPageControl = m_settings.value("MIDI/PrevPageControl", DEFAULT_PREV_PAGE_CONTROL).toInt();
    m_midiDevice = m_settings.value("MIDI/Device", "").toString();
}

void Settings::resetToDefaults()
{
    // General settings
    setAutoOpenLast(DEFAULT_AUTO_OPEN_LAST);
    setDefaultZoom(DEFAULT_ZOOM);
    setDefaultViewMode(DEFAULT_VIEW_MODE);

    // MIDI settings
    setMidiChannel(DEFAULT_MIDI_CHANNEL);
    setNextPageControl(DEFAULT_NEXT_PAGE_CONTROL);
    setPrevPageControl(DEFAULT_PREV_PAGE_CONTROL);
    setMidiDevice("");
}
