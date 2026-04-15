#include "EntreeStock.h"

EntreeStock::EntreeStock()
    : m_entreeStockId(QUuid::createUuid()),
      m_quantite(0),
      m_prixUnitaire(0.0),
      m_statutValidation("EN_ATTENTE"),
      m_date(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

EntreeStock::~EntreeStock()
{
}

bool EntreeStock::estValide() const
{
    return !m_produitId.isNull() 
        && !m_sourceEntreeId.isNull() 
        && m_quantite > 0 
        && !m_creePar.isNull();
}