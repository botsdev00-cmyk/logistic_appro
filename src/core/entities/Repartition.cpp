#include "Repartition.h"

Repartition::Repartition()
    : m_repartitionId(QUuid::createUuid()),
      m_statut(Statut::EnAttente),
      m_dateRepartition(QDate::currentDate()),
      m_montantCashAttendu(0.0),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

Repartition::~Repartition()
{
}

QString Repartition::statutToString(Statut statut)
{
    switch (statut) {
        case Statut::EnAttente:
            return "en_attente";
        case Statut::EnCours:
            return "en_cours";
        case Statut::Completee:
            return "completee";
        case Statut::Annulee:
            return "annulee";
        default:
            return "en_attente";
    }
}

Repartition::Statut Repartition::stringToStatut(const QString& str)
{
    QString lower = str.toLower();
    if (lower == "en_cours") return Statut::EnCours;
    if (lower == "completee") return Statut::Completee;
    if (lower == "annulee") return Statut::Annulee;
    return Statut::EnAttente;
}

QString Repartition::getStatutLabel() const
{
    switch (m_statut) {
        case Statut::EnAttente:
            return "En attente";
        case Statut::EnCours:
            return "En cours";
        case Statut::Completee:
            return "Complétée";
        case Statut::Annulee:
            return "Annulée";
        default:
            return "Inconnu";
    }
}