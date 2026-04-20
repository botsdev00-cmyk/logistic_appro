#ifndef REPOSITORYARTICLEREPARTITION_H
#define REPOSITORYARTICLEREPARTITION_H

#include "../../core/entities/ArticleRepartition.h"
#include <QList>
#include <QUuid>

class RepositoryArticleRepartition {
public:
    RepositoryArticleRepartition();
    bool create(const ArticleRepartition& article);
    QList<ArticleRepartition> getByRepartitionId(const QUuid& repartitionId);
    QString getLastError() const { return m_dernierErreur; }
private:
    QString m_dernierErreur;
};

#endif // REPOSITORYARTICLEREPARTITION_H