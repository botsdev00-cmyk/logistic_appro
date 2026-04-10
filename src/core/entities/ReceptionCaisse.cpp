#include "ReceptionCaisse.h"

ReceptionCaisse::ReceptionCaisse()
    : m_receptionCaisseId(QUuid::createUuid()),
      m_montantAttendu(0.0),
      m_montantRecu(0.0),
      m_statut(Statut::EnAttente),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

ReceptionCaisse::~ReceptionCaisse()
{
}

QString ReceptionCaisse::statutToString(Statut statut)
{
    switch (statut) {
        case Statut::EnAttente:
            return "en_attente";
        case Statut::Recu:
            return "recu";
        case Statut::Valide:
            return "validé";
        case Statut::Discrepance:
            return "discrepance";
        default:
            return "en_attente";
    }
}

ReceptionCaisse::Statut ReceptionCaisse::stringToStatut(const QString& str)
{
    QString lower = str.toLower();
    if (lower == "recu") return Statut::Recu;
    if (lower == "validé") return Statut::Valide;
    if (lower == "discrepance") return Statut::Discrepance;
    return Statut::EnAttente;
}

QString ReceptionCaisse::getStatutLabel() const
{
    switch (m_statut) {
        case Statut::EnAttente:
            return "En attente";
        case Statut::Recu:
            return "Reçu";
        case Statut::Valide:
            return "Validé";
        case Statut::Discrepance:
            return "Discordance";
        default:
            return "Inconnu";
    }
}

bool ReceptionCaisse::hasDiscrepancy() const
{
    return getEcart() != 0.0;
}