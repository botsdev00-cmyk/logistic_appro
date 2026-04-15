#include "RetourStock.h"

RetourStock::RetourStock()
    : m_retourStockId(QUuid::createUuid()),
      m_quantite(0),
      m_statutValidation("EN_ATTENTE"),
      m_date(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

RetourStock::~RetourStock()
{
}

bool RetourStock::estValide() const
{
    return !m_produitId.isNull() 
        && !m_raisonRetourId.isNull() 
        && m_quantite > 0 
        && !m_creePar.isNull();
}