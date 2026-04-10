#include "Credit.h"

Credit::Credit()
    : m_creditId(QUuid::createUuid()),
      m_montant(0.0),
      m_dateEcheance(QDate::currentDate()),
      m_statut(Statut::EnAttente),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

Credit::~Credit()
{
}

QString Credit::statutToString(Statut statut)
{
    switch (statut) {
        case Statut::EnAttente:
            return "en_attente";
        case Statut::Paye:
            return "payé";
        case Statut::EnRetard:
            return "en_retard";
        case Statut::Annule:
            return "annulé";
        default:
            return "en_attente";
    }
}

Credit::Statut Credit::stringToStatut(const QString& str)
{
    QString lower = str.toLower();
    if (lower == "payé") return Statut::Paye;
    if (lower == "en_retard") return Statut::EnRetard;
    if (lower == "annulé") return Statut::Annule;
    return Statut::EnAttente;
}

QString Credit::getStatutLabel() const
{
    switch (m_statut) {
        case Statut::EnAttente:
            return "En attente";
        case Statut::Paye:
            return "Payé";
        case Statut::EnRetard:
            return "En retard";
        case Statut::Annule:
            return "Annulé";
        default:
            return "Inconnu";
    }
}

int Credit::getJoursRetard() const
{
    return QDate::currentDate().daysTo(m_dateEcheance);
}

bool Credit::estEnRetard() const
{
    return QDate::currentDate() > m_dateEcheance && m_statut == Statut::EnAttente;
}