#include "ArticleRepartition.h"
#include "../../utils/globals/globals.h"

ArticleRepartition::ArticleRepartition()
    : m_articleRepartitionId(QUuid::createUuid()),
    m_repartitionId(QUuid()),
    m_produitId(QUuid()),
    m_quantiteVente(0),
    m_quantiteCadeau(0),
    m_quantiteDegustation(0),
    m_observation(""),
    m_syncStatus(SyncStatus::PENDING),
    m_version(1),
    m_deletedAt(QDateTime()),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime()),
    m_createdBy(QUuid()),
    m_updatedBy(QUuid())
{

}

ArticleRepartition::ArticleRepartition(const QUuid& utilisateurCourant)
    : ArticleRepartition()
{
    m_createdBy = utilisateurCourant;
    m_updatedBy = utilisateurCourant;
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
