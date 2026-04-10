#ifndef REPOSITORYCATEGORIEPRODUIT_H
#define REPOSITORYCATEGORIEPRODUIT_H

#include "IRepository.h"
#include "../../core/entities/CategorieProduit.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryCategorieProduit : public IRepository<CategorieProduit>
{
public:
    RepositoryCategorieProduit();

    bool create(const CategorieProduit& entity) override;
    CategorieProduit getById(const QUuid& id) override;
    QList<CategorieProduit> getAll() override;
    bool update(const CategorieProduit& entity) override;
    bool remove(const QUuid& id) override;

    QList<CategorieProduit> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    CategorieProduit getByCode(const QString& code);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYCATEGORIEPRODUIT_H