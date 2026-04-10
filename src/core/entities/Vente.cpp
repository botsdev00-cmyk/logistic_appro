#include "Vente.h"

Vente::Vente()
    : m_venteId(QUuid::createUuid()),
      m_quantite(0),
      m_typeVente(TypeVente::Cash),
      m_prixUnitaire(0.0),
      m_dateVente(QDateTime::currentDateTime())
{
}

Vente::~Vente()
{
}

QString Vente::typeVenteToString(TypeVente type)
{
    switch (type) {
        case TypeVente::Cash:
            return "cash";
        case TypeVente::Credit:
            return "credit";
        default:
            return "cash";
    }
}

Vente::TypeVente Vente::stringToTypeVente(const QString& str)
{
    return str.toLower() == "credit" ? TypeVente::Credit : TypeVente::Cash;
}

QString Vente::getTypeVenteLabel() const
{
    return m_typeVente == TypeVente::Cash ? "Espèces" : "Crédit";
}