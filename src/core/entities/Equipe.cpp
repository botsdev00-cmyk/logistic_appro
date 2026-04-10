#include "Equipe.h"

Equipe::Equipe()
    : m_equipeId(QUuid::createUuid()),
      m_estActif(true),
      m_dateCreation(QDateTime::currentDateTime())
{
}

Equipe::~Equipe()
{
}