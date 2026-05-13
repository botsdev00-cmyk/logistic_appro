#include "CategorieProduit.h"

CategorieProduit::CategorieProduit()
    : m_categorieProduitId(QUuid::createUuid()),
    m_estActif(true),
    m_ordreAffichage(0),
    m_dateCreation(QDateTime::currentDateTime()),
    m_dateMiseAJour(QDateTime::currentDateTime()),
    m_version(1),
    m_syncStatus(SyncStatus::PENDING)
{}

CategorieProduit::~CategorieProduit() {}

QString CategorieProduit::syncStatusString() const
{
    switch(m_syncStatus) {
    case SyncStatus::PENDING: return "PENDING";
    case SyncStatus::SYNCED: return "SYNCED";
    case SyncStatus::CONFLICT: return "CONFLICT";
    default: return "PENDING";
    }
}

CategorieProduit::SyncStatus CategorieProduit::fromString(const QString& v)
{
    if (v == "SYNCED") return SyncStatus::SYNCED;
    if (v == "CONFLICT") return SyncStatus::CONFLICT;
    return SyncStatus::PENDING;
}
