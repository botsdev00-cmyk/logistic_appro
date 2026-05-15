#include "Client.h"

Client::Client()
    : m_clientId(QUuid::createUuid()),
    m_routeId(QUuid()),
    m_conditionPaiementId(QUuid()),
    m_version(1),
    m_syncState(SyncState::PENDING),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime())
{}

Client::~Client() {}

QString Client::syncStateString() const
{
    switch (m_syncState) {
    case SyncState::PENDING:  return "PENDING";
    case SyncState::SYNCED:   return "SYNCED";
    case SyncState::CONFLICT: return "CONFLICT";
    default:                  return "PENDING";
    }
}
SyncState Client::syncStateFromString(const QString& val)
{
    QString v = val.trimmed().toUpper();
    if (v == "SYNCED")   return SyncState::SYNCED;
    if (v == "CONFLICT") return SyncState::CONFLICT;
    return SyncState::PENDING;
}
