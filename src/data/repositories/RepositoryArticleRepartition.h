#ifndef REPOSITORYARTICLEREPARTITION_H
#define REPOSITORYARTICLEREPARTITION_H

#include "../../core/entities/ArticleRepartition.h"
#include <QList>
#include <QUuid>
#include <QString>
#include <QSqlQuery>

class RepositoryArticleRepartition
{
public:
    RepositoryArticleRepartition();

    bool create(const ArticleRepartition& article);
    bool update(const ArticleRepartition& article);
    bool remove(const QUuid& articleId);
    ArticleRepartition getById(const QUuid& id) const;
    QList<ArticleRepartition> getAll() const;
    QList<ArticleRepartition> getByRepartitionId(const QUuid& repartitionId) const;

    // Offline-first utilities
    QList<ArticleRepartition> getPendingSync() const;
    QList<ArticleRepartition> getSyncedArticles() const;
    QList<ArticleRepartition> getConflictSync() const;
    QList<ArticleRepartition> getSinceVersion(int minVersion) const;
    int getPendingCount() const;

    struct SyncResult {
        QUuid articleId;
        bool success;
        QString message;
        int newVersion;
    };
    QList<SyncResult> syncBatch(const QList<ArticleRepartition>& articles, const QUuid& utilisateurId);

    QString getLastError() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    ArticleRepartition mapRowToArticle(const QSqlQuery& query) const;
};

#endif // REPOSITORYARTICLEREPARTITION_H
