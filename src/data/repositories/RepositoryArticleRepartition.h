#ifndef REPOSITORYARTICLEREPARTITION_H
#define REPOSITORYARTICLEREPARTITION_H

#include "../../core/entities/ArticleRepartition.h"
#include <QList>
#include <QUuid>
#include <optional>

class RepositoryArticleRepartition {
public:
    RepositoryArticleRepartition();

    // CRUD standardisé
    bool create(const ArticleRepartition& article);                  
    bool update(const ArticleRepartition& article);                  
    bool logicalDelete(const QUuid& articleRepartitionId);           
    std::optional<ArticleRepartition> getById(const QUuid& id) const;
    QList<ArticleRepartition> getAll() const;
    QList<ArticleRepartition> getByRepartitionId(const QUuid& repartitionId) const;

    // OFFLINE-FIRST
    QList<ArticleRepartition> getPendingSync() const;                
    QList<ArticleRepartition> getSinceVersion(int minVersion) const;

    QString getLastError() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYARTICLEREPARTITION_H