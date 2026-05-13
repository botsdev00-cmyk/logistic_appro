#include "Produit.h"

Produit::Produit()
    : m_produitId(QUuid::createUuid()),
    m_prixUnitaire(0.0),
    m_stockMinimum(0),
    m_estActif(true),
    m_dateCreation(QDateTime::currentDateTime()),
    m_dateMiseAJour(QDateTime::currentDateTime()),
    m_version(1),
    m_syncStatus(SyncStatus::PENDING)
{}

Produit::~Produit()
{}

QString Produit::syncStatusString() const
{
    switch(m_syncStatus) {
    case SyncStatus::PENDING: return "PENDING";
    case SyncStatus::SYNCED: return "SYNCED";
    case SyncStatus::CONFLICT: return "CONFLICT";
    default: return "PENDING";
    }
}

Produit::SyncStatus Produit::fromString(const QString& v)
{
    if (v == "SYNCED") return SyncStatus::SYNCED;
    if (v == "CONFLICT") return SyncStatus::CONFLICT;
    return SyncStatus::PENDING;
}
