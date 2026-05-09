#include "GestionnaireEquipe.h"
#include "../../data/repositories/RepositoryEquipe.h"
#include <QDebug>

GestionnaireEquipe::GestionnaireEquipe()
{
}

GestionnaireEquipe::~GestionnaireEquipe()
{
}

QUuid GestionnaireEquipe::creerEquipe(const QString& nom, 
                                      const QString& nomChef,
                                      const QString& telephoneChef,
                                      const QString& description,
                                      const QUuid& createdBy)
{
    m_dernierErreur.clear();

    if (nom.isEmpty()) {
        m_dernierErreur = "Le nom de l'équipe ne peut pas être vide";
        return QUuid();
    }

    Equipe equipe;
    equipe.setEquipeId(QUuid::createUuid());
    equipe.setNom(nom);
    equipe.setNomChef(nomChef);
    equipe.setTelephoneChef(telephoneChef);
    equipe.setDescription(description);
    equipe.setCreatedBy(createdBy);
    equipe.setUpdatedBy(createdBy);
    equipe.setSyncStatus(Equipe::PENDING);
    equipe.setVersion(1);
    equipe.setEstActif(true);

    RepositoryEquipe repo;
    if (!repo.create(equipe)) {
        m_dernierErreur = repo.getLastError();
        return QUuid();
    }

    qDebug() << "GestionnaireEquipe::creerEquipe success:" << nom << "(" << equipe.getEquipeId() << ")";
    return equipe.getEquipeId();
}

bool GestionnaireEquipe::modifierEquipe(const Equipe& equipe, const QUuid& updatedBy)
{
    m_dernierErreur.clear();

    if (equipe.getNom().isEmpty()) {
        m_dernierErreur = "Le nom de l'équipe ne peut pas être vide";
        return false;
    }

    Equipe equipeToUpdate = equipe;
    equipeToUpdate.setUpdatedBy(updatedBy);
    equipeToUpdate.setSyncStatus(Equipe::PENDING);
    equipeToUpdate.setVersion(equipeToUpdate.getVersion() + 1);
    equipeToUpdate.setUpdatedAt(QDateTime::currentDateTime());

    RepositoryEquipe repo;
    if (!repo.update(equipeToUpdate)) {
        m_dernierErreur = repo.getLastError();
        return false;
    }

    qDebug() << "GestionnaireEquipe::modifierEquipe success:" << equipe.getNom();
    return true;
}

bool GestionnaireEquipe::supprimerEquipe(const QUuid& equipeId, const QUuid& deletedBy)
{
    m_dernierErreur.clear();

    RepositoryEquipe repo;
    Equipe equipe = repo.getById(equipeId);
    
    if (equipe.getEquipeId().isNull()) {
        m_dernierErreur = "Équipe non trouvée";
        return false;
    }

    if (!repo.remove(equipeId)) {
        m_dernierErreur = repo.getLastError();
        return false;
    }

    qDebug() << "GestionnaireEquipe::supprimerEquipe success:" << equipeId;
    return true;
}

Equipe GestionnaireEquipe::obtenirEquipe(const QUuid& equipeId) const
{
    RepositoryEquipe repo;
    return repo.getById(equipeId);
}

QList<Equipe> GestionnaireEquipe::listerEquipes() const
{
    RepositoryEquipe repo;
    return repo.getAll();
}

QList<Equipe> GestionnaireEquipe::rechercherEquipes(const QString& criterion) const
{
    RepositoryEquipe repo;
    return repo.search(criterion);
}

QList<Equipe> GestionnaireEquipe::obtenirEquipesPendantes() const
{
    RepositoryEquipe repo;
    return repo.getPendingEquipes();
}

QList<Equipe> GestionnaireEquipe::obtenirEquipesEnConflit() const
{
    RepositoryEquipe repo;
    return repo.getConflictEquipes();
}

int GestionnaireEquipe::compterEquipesPendantes() const
{
    RepositoryEquipe repo;
    return repo.getPendingCount();
}

GestionnaireEquipe::SyncReport GestionnaireEquipe::synchroniserEquipes(
    const QList<Equipe>& equipes, const QUuid& utilisateurId)
{
    m_dernierErreur.clear();
    SyncReport report;
    report.totalTreated = equipes.size();
    report.syncedCount = 0;
    report.conflictCount = 0;

    if (equipes.isEmpty()) {
        report.message = "Aucune équipe à synchroniser";
        return report;
    }

    RepositoryEquipe repo;
    QList<RepositoryEquipe::SyncResult> results = repo.syncBatch(equipes, utilisateurId);

    for (const auto& result : results) {
        if (result.success) {
            report.syncedCount++;
            qDebug() << "Sync success:" << result.equipeId.toString() << "v" << result.newVersion;
        } else {
            if (result.message.contains("CONFLICT")) {
                report.conflictCount++;
            }
            qDebug() << "Sync failed:" << result.message;
        }
    }

    report.message = QString("Sync report: %1/%2 synced, %3 conflicts")
        .arg(report.syncedCount)
        .arg(report.totalTreated)
        .arg(report.conflictCount);

    qDebug() << "GestionnaireEquipe::synchroniserEquipes" << report.message;
    return report;
}
