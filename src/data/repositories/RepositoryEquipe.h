#ifndef REPOSITORYEQUIPE_H
#define REPOSITORYEQUIPE_H

#include "IRepository.h"
#include "../entities/Equipe.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryEquipe : public IRepository<Equipe>
{
public:
    RepositoryEquipe();

    bool create(const Equipe& entity) override;
    Equipe getById(const QUuid& id) override;
    QList<Equipe> getAll() override;
    bool update(const Equipe& entity) override;
    bool remove(const QUuid& id) override;

    QList<Equipe> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYEQUIPE_H