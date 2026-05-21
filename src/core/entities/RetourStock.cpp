#include "RetourStock.h"

RetourStock::RetourStock()
    : m_retourStockId(QUuid::createUuid()),
    m_quantite(0),
    m_syncStatus(PENDING),
    m_version(1),
    m_statutValidation("EN_ATTENTE"),
    m_date(QDateTime::currentDateTime()),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime()),
    m_dateMiseAJour(QDateTime::currentDateTime())
{
}

RetourStock::~RetourStock() {}

bool RetourStock::estValide() const {
    return !m_produitId.isNull()
    && !m_raisonRetourId.isNull()
        && m_quantite > 0
        && !m_creePar.isNull();
}

QString RetourStock::syncStatusString() const
{
    switch (m_syncStatus) {
    case PENDING:   return "PENDING";
    case SYNCED:    return "SYNCED";
    case CONFLICT:  return "CONFLICT";
    default:        return "PENDING";
    }
}

RetourStock::SyncStatus RetourStock::stringToSyncStatus(const QString& str)
{
    if (str == "PENDING")   return PENDING;
    if (str == "SYNCED")    return SYNCED;
    if (str == "CONFLICT")  return CONFLICT;
    return PENDING;
}
