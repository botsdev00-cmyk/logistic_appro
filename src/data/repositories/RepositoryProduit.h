#ifndef REPOSITORYPRODUIT_H
#define REPOSITORYPRODUIT_H

#include "IRepository.h"
#include "../../core/entities/Produit.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryProduit : public IRepository<Produit>
{
public:
    RepositoryProduit();

    bool create(const Produit& entity) override;
    Produit getById(const QUuid& id) override;
    QList<Produit> getAll() override;
    bool update(const Produit& entity) override;
    bool remove(const QUuid& id) override;

    QList<Produit> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    Produit getByCodeSku(const QString& sku);
    QList<Produit> getByCategorie(const QUuid& categorieId);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYPRODUIT_H