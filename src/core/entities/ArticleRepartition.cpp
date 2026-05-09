#include "ArticleRepartition.h"

ArticleRepartition::ArticleRepartition()
    : m_quantiteVente(0),
      m_quantiteCadeau(0),
      m_quantiteDegustation(0),
      m_syncStatus(PENDING),
      m_version(1),
      m_createdAt(QDateTime::currentDateTime()),
      m_updatedAt(QDateTime::currentDateTime())
{
    m_articleRepartitionId = QUuid::createUuid();
}

ArticleRepartition::~ArticleRepartition()
{
}

QString ArticleRepartition::syncStatusString() const
{
    switch (m_syncStatus) {
        case PENDING:
            return "PENDING";
        case SYNCED:
            return "SYNCED";
        case CONFLICT:
            return "CONFLICT";
        default:
            return "UNKNOWN";
    }
}

ArticleRepartition::SyncStatus ArticleRepartition::stringToSyncStatus(const QString& str)
{
    if (str == "PENDING") return PENDING;
    if (str == "SYNCED") return SYNCED;
    if (str == "CONFLICT") return CONFLICT;
    return PENDING;
}
