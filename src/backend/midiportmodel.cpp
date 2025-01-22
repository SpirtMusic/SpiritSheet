#include "midiportmodel.h"

MidiPortModel::MidiPortModel(QObject *parent)
    : QAbstractListModel{parent}
{}
int MidiPortModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_ports.count();
}

QVariant MidiPortModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_ports.size())
        return QVariant();

    const PortInfo &info = m_ports[index.row()];
    switch (role) {
    case NameRole:
        return info.name;
    case PortRole:
        return info.port;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MidiPortModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PortRole] = "port";
    return roles;
}

void MidiPortModel::addPort(const QString &name, const QVariant &port)
{
    if (!name.startsWith("VSpiritMusic", Qt::CaseInsensitive))
    {
        beginInsertRows(QModelIndex(), m_ports.size(), m_ports.size());
        m_ports.append({name, port});
        endInsertRows();
        Q_EMIT countChanged();
    }
}

void MidiPortModel::clear()
{
    beginResetModel();
    m_ports.clear();
    endResetModel();
    Q_EMIT countChanged();
}
