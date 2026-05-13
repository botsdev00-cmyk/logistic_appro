#include "ReceptionCaisse.h"

ReceptionCaisse::ReceptionCaisse()
    : m_receptionCaisseId(QUuid::createUuid()),
    m_montantAttendu(0.0),
    m_montantRecu(0.0),
    m_statut(Statut::EnAttente),
    m_dateCreation(QDateTime::currentDateTime()),
    m_dateMiseAJour(QDateTime::currentDateTime()),
    m_deletedAt(),
    m_version(1),
    m_syncStatus(SyncStatus::PENDING)
{}

ReceptionCaisse::~ReceptionCaisse() {}

QString ReceptionCaisse::statutToString(Statut s) {
    switch (s) {
    case Statut::EnAttente: return "EN_ATTENTE";
    case Statut::Recu: return "RECU";
    case Statut::Valide: return "VALIDE";
    case Statut::Discrepance: return "DISCREPANCE";
    default: return "EN_ATTENTE";
    }
}

ReceptionCaisse::Statut ReceptionCaisse::stringToStatut(const QString& str) {
    QString val = str.trimmed().toUpper();
    if (val == "RECU") return Statut::Recu;
    if (val == "VALIDE") return Statut::Valide;
    if (val == "DISCREPANCE") return Statut::Discrepance;
    return Statut::EnAttente;
}

QString ReceptionCaisse::syncStatusToString(SyncStatus s) {
    switch (s) {
    case SyncStatus::PENDING: return "PENDING";
    case SyncStatus::SYNCED: return "SYNCED";
    case SyncStatus::CONFLICT: return "CONFLICT";
    default: return "PENDING";
    }
}

ReceptionCaisse::SyncStatus ReceptionCaisse::stringToSyncStatus(const QString& str) {
    QString val = str.trimmed().toUpper();
    if (val == "SYNCED") return SyncStatus::SYNCED;
    if (val == "CONFLICT") return SyncStatus::CONFLICT;
    return SyncStatus::PENDING;
}

QString ReceptionCaisse::getStatutLabel() const {
    switch (m_statut) {
    case Statut::EnAttente: return "En attente";
    case Statut::Recu: return "Reçu";
    case Statut::Valide: return "Validé";
    case Statut::Discrepance: return "Discordance";
    default: return "Inconnu";
    }
}
