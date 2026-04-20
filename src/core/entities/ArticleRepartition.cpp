#include "ArticleRepartition.h"

ArticleRepartition::ArticleRepartition()
    : m_articleRepartitionId(QUuid::createUuid()),
      m_quantiteVente(0),
      m_quantiteCadeau(0),
      m_quantiteDegustation(0)
{
}
ArticleRepartition::~ArticleRepartition() {}