#include "CategorieProduit.h"

CategorieProduit::CategorieProduit()
    : m_categorieProduitId(QUuid::createUuid()),
      m_estActif(true),
      m_ordreAffichage(0),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

CategorieProduit::~CategorieProduit()
{
}