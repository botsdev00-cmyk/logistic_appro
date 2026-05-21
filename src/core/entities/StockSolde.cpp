#include "StockSolde.h"

StockSolde::StockSolde()
    : m_soldeId(QUuid::createUuid()),
    m_quantiteTotal(0),
    m_quantiteReserve(0),
    m_quantiteDisponible(0),
    m_valeurStock(0.0),
    m_prixMoyen(0.0),
    m_locationId("WAREHOUSE"),
    m_derniereLocationId("WAREHOUSE"),
    m_historyReturned(0),
    m_historyWarehouse(0),
    m_historyInTransit(0),
    m_dernierMouvementDate(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime()),
    m_createdAt(QDateTime::currentDateTime()),
    m_syncStatus(PENDING),
    m_version(1),
    m_isDeleted(false)
{}

StockSolde::~StockSolde() {}

// Helpers
QString StockSolde::syncStatusString() const
{
    switch (m_syncStatus) {
    case PENDING: return "PENDING";
    case SYNCED: return "SYNCED";
    case CONFLICT: return "CONFLICT";
    default: return "UNKNOWN";
    }
}

StockSolde::SyncStatus StockSolde::stringToSyncStatus(const QString& str)
{
    if (str == "PENDING") return PENDING;
    if (str == "SYNCED") return SYNCED;
    if (str == "CONFLICT") return CONFLICT;
    return PENDING;
}
