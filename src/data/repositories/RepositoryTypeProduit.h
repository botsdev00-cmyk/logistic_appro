#ifndef REPOSITORYTYPEPRODUIT_H
#define REPOSITORYTYPEPRODUIT_H

#include <QList>
#include <QUuid>
#include <QString>

struct TypeProduit {
    QUuid id;
    QString code;
    QString nom;
};

class RepositoryTypeProduit {
public:
    QList<TypeProduit> getAll();
    bool create(const QString& nom, const QString& code);
    QString getLastError() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif