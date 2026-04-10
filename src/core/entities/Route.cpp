#include "Route.h"

Route::Route()
    : m_routeId(QUuid::createUuid()),
      m_estActif(true),
      m_dateCreation(QDateTime::currentDateTime())
{
}

Route::~Route()
{
}