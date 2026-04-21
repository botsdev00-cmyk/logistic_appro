#include "GestionnaireEquipe.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include "../../core/entities/Equipe.h"

QUuid GestionnaireEquipe::creerEquipe(const QString& nom, const QUuid& chefId, const QList<QUuid>& membres, const QString& telephoneChef)
{
    m_dernierErreur.clear();

    Equipe equipe;
    equipe.setEquipeId(QUuid::createUuid());
    equipe.setNom(nom);
    equipe.setNomChef(chefId.toString()); // Utilisateur ID sous forme QString (à adapter si ta colonne attend autre chose)
    // equipe.setEstActif(true);

    RepositoryEquipe repo;
    if (!repo.create(equipe)) {
        m_dernierErreur = repo.getLastError();
        return QUuid();
    }
    // Optionnel : ajout membres dans une table de jointure ici si tu as une table equipe_membres ! (à implémenter selon ton modèle)

    // Tu pourrais faire un getById pour récupérer les infos : 
    // Equipe eq = repo.getById(equipe.getEquipeId());

    return equipe.getEquipeId();
}