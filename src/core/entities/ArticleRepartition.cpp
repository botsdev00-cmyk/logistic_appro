#include "ArticleRepartition.h"

ArticleRepartition::ArticleRepartition()
    : m_quantiteVente(0),
    m_quantiteCadeau(0),
    m_quantiteDegustation(0),
    m_syncStatus(SyncStatus::PENDING),
    m_version(1),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime())
{
    m_articleRepartitionId = QUuid::createUuid();
}

ArticleRepartition::~ArticleRepartition() {}

QString ArticleRepartition::syncStatusString() const
{
    switch (m_syncStatus) {
    case SyncStatus::PENDING: return "PENDING";
    case SyncStatus::SYNCED:  return "SYNCED";
    case SyncStatus::CONFLICT: return "CONFLICT";
    default: return "PENDING";
    }
}

ArticleRepartition::SyncStatus ArticleRepartition::stringToSyncStatus(const QString& str)
{
    QString t = str.trimmed().toUpper();
    if (t == "PENDING") return SyncStatus::PENDING;
    if (t == "SYNCED")  return SyncStatus::SYNCED;
    if (t == "CONFLICT") return SyncStatus::CONFLICT;
    return SyncStatus::PENDING;
}
