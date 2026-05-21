#include "Repartition.h"

Repartition::Repartition()
    : m_statut(Statut::EnAttente),
    m_montantCashAttendu(0.0),
    m_annule(false),
    m_mouvementsGeneres(false),
    m_syncStatus("PENDING"),
    m_version(1),
    m_createdAt(QDateTime::currentDateTime()),
    m_updatedAt(QDateTime::currentDateTime())
{
    m_repartitionId = QUuid::createUuid();
}

Repartition::~Repartition() {}

QString Repartition::statutToString(Statut s)
{
    switch (s) {
    case Statut::EnAttente: return "EN_ATTENTE";
    case Statut::EnCours: return "EN_COURS";
    case Statut::Completee: return "COMPLETEE";
    case Statut::Annulee: return "ANNULEE";
    default: return "EN_ATTENTE";
    }
}

Repartition::Statut Repartition::stringToStatut(const QString& str)
{
    QString v = str.trimmed().toUpper();
    if (v == "EN_COURS") return Statut::EnCours;
    if (v == "COMPLETEE") return Statut::Completee;
    if (v == "ANNULEE") return Statut::Annulee;
    return Statut::EnAttente;
}

QString Repartition::getStatutLabel() const
{
    switch (m_statut) {
    case Statut::EnAttente: return "En attente";
    case Statut::EnCours: return "En cours";
    case Statut::Completee: return "Complétée";
    case Statut::Annulee: return "Annulée";
    default: return "Inconnu";
    }
}
