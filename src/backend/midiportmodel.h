#ifndef MIDIPORTMODEL_H
#define MIDIPORTMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QObject>

class MidiPortModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit MidiPortModel(QObject *parent = nullptr);
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PortRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addPort(const QString &name, const QVariant &port);
    void clear();
signals:
    void countChanged();
private:
    struct PortInfo {
        QString name;
        QVariant port;
    };
    QList<PortInfo> m_ports;
};

#endif // MIDIPORTMODEL_H
