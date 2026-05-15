#include "Equipe.h"

Equipe::Equipe()
    : m_equipeId(QUuid::createUuid()),
    m_estActif(true),
    m_syncStatus(SyncStatus::PENDING),
    m_version(1),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime())
{}

Equipe::~Equipe() {}

QString Equipe::syncStatusString() const
{
    switch (m_syncStatus) {
    case SyncStatus::PENDING:  return "PENDING";
    case SyncStatus::SYNCED:   return "SYNCED";
    case SyncStatus::CONFLICT: return "CONFLICT";
    default:                   return "UNKNOWN";
    }
}

Equipe::SyncStatus Equipe::syncStatusFromString(const QString& s)
{
    QString v = s.trimmed().toUpper();
    if (v == "SYNCED")   return SyncStatus::SYNCED;
    if (v == "CONFLICT") return SyncStatus::CONFLICT;
    return SyncStatus::PENDING;
}
