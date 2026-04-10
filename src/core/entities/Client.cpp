#include "Client.h"

Client::Client()
    : m_clientId(QUuid::createUuid()),
      m_conditionsPaiement(ConditionsPaiement::Cash),
      m_estActif(true),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

Client::~Client()
{
}

QString Client::conditionsPaiementToString(ConditionsPaiement conditions)
{
    switch (conditions) {
        case ConditionsPaiement::Cash:
            return "cash";
        case ConditionsPaiement::Credit5Jours:
            return "credit_5jours";
        case ConditionsPaiement::Credit7Jours:
            return "credit_7jours";
        default:
            return "cash";
    }
}

Client::ConditionsPaiement Client::stringToConditionsPaiement(const QString& str)
{
    QString lower = str.toLower();
    if (lower == "credit_5jours") return ConditionsPaiement::Credit5Jours;
    if (lower == "credit_7jours") return ConditionsPaiement::Credit7Jours;
    return ConditionsPaiement::Cash;
}

QString Client::getConditionsPaiementLabel() const
{
    switch (m_conditionsPaiement) {
        case ConditionsPaiement::Cash:
            return "Espèces";
        case ConditionsPaiement::Credit5Jours:
            return "Crédit 5 jours";
        case ConditionsPaiement::Credit7Jours:
            return "Crédit 7 jours";
        default:
            return "Inconnu";
    }
}