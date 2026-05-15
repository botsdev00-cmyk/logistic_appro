#include "Credit.h"

Credit::Credit()
    : m_creditId(QUuid::createUuid()),
    m_montant(0.0),
    m_dateEcheance(QDate::currentDate()),
    m_statut(Statut::EnAttente),
    m_version(1),
    m_syncStatus(SyncStatus::PENDING),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime())
{}

Credit::~Credit() {}

QString Credit::statutToString(Statut statut)
{
    switch (statut) {
    case Statut::EnAttente: return "EN_ATTENTE";
    case Statut::Paye:      return "PAYE";
    case Statut::EnRetard:  return "EN_RETARD";
    case Statut::Annule:    return "ANNULE";
    default:                return "EN_ATTENTE";
    }
}
Credit::Statut Credit::stringToStatut(const QString& str)
{
    QString s = str.trimmed().toUpper();
    if (s == "PAYE") return Statut::Paye;
    if (s == "EN_RETARD") return Statut::EnRetard;
    if (s == "ANNULE") return Statut::Annule;
    return Statut::EnAttente;
}
QString Credit::getStatutLabel() const
{
    switch (m_statut) {
    case Statut::EnAttente: return "En attente";
    case Statut::Paye:      return "Payé";
    case Statut::EnRetard:  return "En retard";
    case Statut::Annule:    return "Annulé";
    default:                return "Inconnu";
    }
}

int Credit::getJoursRetard() const
{
    return m_dateEcheance.daysTo(QDate::currentDate());
}

bool Credit::estEnRetard() const
{
    return QDate::currentDate() > m_dateEcheance && m_statut == Statut::EnAttente;
}

QString Credit::syncStatusToString(SyncStatus s)
{
    switch (s) {
    case SyncStatus::PENDING:  return "PENDING";
    case SyncStatus::SYNCED:   return "SYNCED";
    case SyncStatus::CONFLICT: return "CONFLICT";
    default:                   return "PENDING";
    }
}
Credit::SyncStatus Credit::syncStatusFromString(const QString& val)
{
    QString v = val.trimmed().toUpper();
    if (v == "SYNCED")   return SyncStatus::SYNCED;
    if (v == "CONFLICT") return SyncStatus::CONFLICT;
    return SyncStatus::PENDING;
}
