#include "StockSolde.h"
#include <QDateTime>

StockSolde::StockSolde()
    : m_soldeId(QUuid::createUuid()),
      m_produitNom(""),
      m_codeSKU(""),
      m_categorie(""),
      m_type(""),
      m_stockMinimum(0),
      m_quantiteTotal(0),
      m_quantiteReservee(0),
      m_valeurStock(0.0),
      m_prixMoyen(0.0),
      m_dernierMouvementDate(QDateTime::currentDateTime()),
      m_updatedAt(QDateTime::currentDateTime())
{
}

StockSolde::~StockSolde()
{
}