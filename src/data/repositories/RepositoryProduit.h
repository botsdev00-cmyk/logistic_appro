#ifndef REPOSITORYPRODUIT_H
#define REPOSITORYPRODUIT_H

#include "IRepository.h"
#include "../../core/entities/Produit.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryProduit {
public:
    RepositoryProduit();

    bool create(const Produit& entity);
    bool update(const Produit& entity);
    bool remove(const QUuid& id);
    std::optional<Produit> getById(const QUuid& id);
    std::optional<Produit> getByCodeSku(const QString& sku);
    QList<Produit> getAll();
    QList<Produit> getByCategorie(const QUuid& categorieId);
    QList<Produit> search(const QString& criterion);
    bool exists(const QUuid& id);
    QString getLastError() const { return m_dernierErreur; }
private:
    mutable QString m_dernierErreur;
};
#endif //REPOSITORYPRODUIT_H
