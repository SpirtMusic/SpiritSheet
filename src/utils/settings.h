#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

class Settings : public QObject
{
    Q_OBJECT

    // General Settings
    Q_PROPERTY(bool autoOpenLast READ autoOpenLast WRITE setAutoOpenLast NOTIFY autoOpenLastChanged)
    Q_PROPERTY(int defaultZoom READ defaultZoom WRITE setDefaultZoom NOTIFY defaultZoomChanged)
    Q_PROPERTY(int defaultViewMode READ defaultViewMode WRITE setDefaultViewMode NOTIFY defaultViewModeChanged)

    // MIDI Settings
    Q_PROPERTY(int midiChannel READ midiChannel WRITE setMidiChannel NOTIFY midiChannelChanged)
    Q_PROPERTY(int nextPageControl READ nextPageControl WRITE setNextPageControl NOTIFY nextPageControlChanged)
    Q_PROPERTY(int prevPageControl READ prevPageControl WRITE setPrevPageControl NOTIFY prevPageControlChanged)
    Q_PROPERTY(QString midiDevice READ midiDevice WRITE setMidiDevice NOTIFY midiDeviceChanged)

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    // General getters
    bool autoOpenLast() const;
    int defaultZoom() const;
    int defaultViewMode() const;

    // MIDI getters
    int midiChannel() const;
    int nextPageControl() const;
    int prevPageControl() const;
    QString midiDevice() const;

    // General setters
    void setAutoOpenLast(bool value);
    void setDefaultZoom(int value);
    void setDefaultViewMode(int value);

    // MIDI setters
    void setMidiChannel(int value);
    void setNextPageControl(int value);
    void setPrevPageControl(int value);
    void setMidiDevice(const QString &device);

    // Load/Save methods
    Q_INVOKABLE void load();
    Q_INVOKABLE void resetToDefaults();

signals:
    // General signals
    void autoOpenLastChanged();
    void defaultZoomChanged();
    void defaultViewModeChanged();

    // MIDI signals
    void midiChannelChanged();
    void nextPageControlChanged();
    void prevPageControlChanged();
    void midiDeviceChanged();

private:
    QSettings m_settings;

    // General settings
    bool m_autoOpenLast;
    int m_defaultZoom;
    int m_defaultViewMode;

    // MIDI settings
    int m_midiChannel;
    int m_nextPageControl;
    int m_prevPageControl;
    QString m_midiDevice;

    // Constants for default values
    static const bool DEFAULT_AUTO_OPEN_LAST = false;
    static const int DEFAULT_ZOOM = 100;
    static const int DEFAULT_VIEW_MODE = 0;
    static const int DEFAULT_MIDI_CHANNEL = 1;
    static const int DEFAULT_NEXT_PAGE_CONTROL = 64;
    static const int DEFAULT_PREV_PAGE_CONTROL = 67;
};

#endif // SETTINGS_H
