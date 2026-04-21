#include "GestionnaireEquipe.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../core/entities/Equipe.h"

QUuid GestionnaireEquipe::creerEquipe(const QString& nom, const QUuid& chefId, const QList<QUuid>& membres, const QString& telephoneChef)
{
    m_dernierErreur.clear();

    Equipe equipe;
    equipe.setEquipeId(QUuid::createUuid());
    equipe.setNom(nom);
    
    // CORRECTION : On formate l'UUID sans les accolades {} pour être 100% compatible avec PostgreSQL
    equipe.setNomChef(chefId.toString(QUuid::WithoutBraces)); 

    RepositoryEquipe repo;
    if (!repo.create(equipe)) {
        m_dernierErreur = repo.getLastError();
        return QUuid();
    }

    return equipe.getEquipeId();
}