#include "Produit.h"

Produit::Produit()
    : m_produitId(QUuid::createUuid()),
      m_prixUnitaire(0.0),
      m_stockMinimum(10),
      m_estActif(true),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

Produit::~Produit()
{
}