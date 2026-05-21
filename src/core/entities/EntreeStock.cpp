#include "EntreeStock.h"

EntreeStock::EntreeStock()
    : m_entreeStockId(QUuid::createUuid()),
    m_quantite(0),
    m_prixUnitaire(0.0),
    m_syncStatus(PENDING),
    m_version(1),
    m_date(QDateTime::currentDateTime()),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime()),
    m_dateMiseAJour(QDateTime::currentDateTime())
{
}

EntreeStock::~EntreeStock()
{
}

bool EntreeStock::estValide() const
{
    // La ligne est considérée comme valide si :
    // - id produit et source_entree non nuls
    // - quantité strictement positive
    // - créateur présent
    return !m_produitId.isNull()
           && !m_sourceEntreeId.isNull()
           && m_quantite > 0
           && !m_creePar.isNull();
}

QString EntreeStock::syncStatusString() const
{
    switch (m_syncStatus) {
    case PENDING:   return "PENDING";
    case SYNCED:    return "SYNCED";
    case CONFLICT:  return "CONFLICT";
    default:        return "PENDING";
    }
}

EntreeStock::SyncStatus EntreeStock::stringToSyncStatus(const QString& str)
{
    if (str == "PENDING")   return PENDING;
    if (str == "SYNCED")    return SYNCED;
    if (str == "CONFLICT")  return CONFLICT;
    return PENDING;
}
