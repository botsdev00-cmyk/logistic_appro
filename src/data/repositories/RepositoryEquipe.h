#ifndef REPOSITORYEQUIPE_H
#define REPOSITORYEQUIPE_H

#include "../../core/entities/Equipe.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>
#include <QSqlQuery>

class RepositoryEquipe
{
public:
    RepositoryEquipe();

    // CRUD
    bool create(const Equipe& entity);
    bool update(const Equipe& entity);
    bool logicalDelete(const QUuid& id);
    std::optional<Equipe> getById(const QUuid& id) const;
    QList<Equipe> getAll() const;

    QList<Equipe> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;

    // Offline-first
    QList<Equipe> getPendingEquipes() const;
    QList<Equipe> getConflictEquipes() const;
    QList<Equipe> getSinceVersion(int minVersion) const;

    QString getLastError() const { return m_dernierErreur; }
private:
    QString m_dernierErreur;
    Equipe mapRowToEquipe(const QSqlQuery& q) const;
};

#endif // REPOSITORYEQUIPE_H
