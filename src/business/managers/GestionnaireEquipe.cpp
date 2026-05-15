#include "GestionnaireEquipe.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include <QDebug>

GestionnaireEquipe::GestionnaireEquipe() {}
GestionnaireEquipe::~GestionnaireEquipe() {}

QUuid GestionnaireEquipe::creerEquipe(const QString& nom, const QString& nomChef, const QUuid& createdBy, bool estActif)
{
    m_dernierErreur.clear();
    if (nom.isEmpty() || nomChef.isEmpty()) {
        m_dernierErreur = "Nom d'équipe ou chef requis";
        return QUuid();
    }
    Equipe equipe;
    equipe.setEquipeId(QUuid::createUuid());
    equipe.setNom(nom);
    equipe.setNomChef(nomChef);
    equipe.setCreatedBy(createdBy);
    equipe.setUpdatedBy(createdBy);
    equipe.setSyncStatus(Equipe::SyncStatus::PENDING);
    equipe.setVersion(1);
    equipe.setEstActif(estActif);
    equipe.setCreatedAt(QDateTime::currentDateTime());
    equipe.setUpdatedAt(QDateTime::currentDateTime());

    RepositoryEquipe repo;
    if (!repo.create(equipe)) {
        m_dernierErreur = repo.getLastError();
        return QUuid();
    }
    return equipe.getEquipeId();
}

bool GestionnaireEquipe::modifierEquipe(const Equipe& input, const QUuid& updatedBy)
{
    m_dernierErreur.clear();
    if (input.getNom().isEmpty()) {
        m_dernierErreur = "Nom d'équipe requis";
        return false;
    }
    Equipe equipe = input;
    equipe.setUpdatedBy(updatedBy);
    equipe.setSyncStatus(Equipe::SyncStatus::PENDING);
    equipe.setVersion(equipe.getVersion() + 1);
    equipe.setUpdatedAt(QDateTime::currentDateTime());

    RepositoryEquipe repo;
    if (!repo.update(equipe)) {
        m_dernierErreur = repo.getLastError();
        return false;
    }
    return true;
}

bool GestionnaireEquipe::supprimerEquipe(const QUuid& equipeId)
{
    RepositoryEquipe repo;
    return repo.logicalDelete(equipeId);
}

std::optional<Equipe> GestionnaireEquipe::obtenirEquipe(const QUuid& equipeId) const
{
    RepositoryEquipe repo;
    return repo.getById(equipeId);
}

QList<Equipe> GestionnaireEquipe::listerEquipes() const
{
    RepositoryEquipe repo;
    return repo.getAll();
}

QList<Equipe> GestionnaireEquipe::rechercherEquipes(const QString& crit) const
{
    RepositoryEquipe repo;
    return repo.search(crit);
}

QList<Equipe> GestionnaireEquipe::obtenirEquipesPendantes() const
{
    RepositoryEquipe repo;
    return repo.getPendingEquipes();
}

QList<Equipe> GestionnaireEquipe::obtenirEquipesConflit() const
{
    RepositoryEquipe repo;
    return repo.getConflictEquipes();
}

QList<Equipe> GestionnaireEquipe::obtenirEquipesDepuisVersion(int version) const
{
    RepositoryEquipe repo;
    return repo.getSinceVersion(version);
}
