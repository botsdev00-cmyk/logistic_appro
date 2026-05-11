#ifndef REPOSITORYEQUIPE_H
#define REPOSITORYEQUIPE_H

#include "IRepository.h"
#include "../../core/entities/Equipe.h"
#include <QList>
#include <QString>
#include <QSqlQuery>
#include <QUuid>

class RepositoryEquipe : public IRepository<Equipe>
{
public:
    RepositoryEquipe();

    // Standard CRUD
    bool create(const Equipe& entity) override;
    Equipe getById(const QUuid& id) override;
    QList<Equipe> getAll() override;
    bool update(const Equipe& entity) override;
    bool remove(const QUuid& id) override;

    // Search & filtering
    QList<Equipe> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    // Offline-first specific methods
    QList<Equipe> getPendingEquipes() const;
    QList<Equipe> getSyncedEquipes() const;
    QList<Equipe> getConflictEquipes() const;
    int getPendingCount() const;

    // Batch sync - C++ API pur
    struct SyncResult {
        QUuid equipeId;
        bool success;
        QString message;
        int newVersion;
    };
    QList<SyncResult> syncBatch(const QList<Equipe>& equipes, const QUuid& utilisateurId);

    // Error handling
    QString getLastError() const override { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    Equipe mapRowToEquipe(const QSqlQuery& query) const;
};

#endif // REPOSITORYEQUIPE_H
