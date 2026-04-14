#include "RepositoryTypeProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>

QList<TypeProduit> RepositoryTypeProduit::getAll() {
    QList<TypeProduit> liste;
    QSqlQuery query("SELECT type_produit_id, code, nom FROM types_produits ORDER BY nom", 
                     ConnexionBaseDonnees::getInstance().getDatabase());
    
    while (query.next()) {
        liste.append({QUuid(query.value(0).toString()), query.value(1).toString(), query.value(2).toString()});
    }
    return liste;
}

bool RepositoryTypeProduit::create(const QString& nom, const QString& code) {
    QSqlQuery query(ConnexionBaseDonnees::getInstance().getDatabase());
    query.prepare("INSERT INTO types_produits (type_produit_id, nom, code) VALUES (:id, :nom, :code)");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":nom", nom);
    query.bindValue(":code", code);
    
    if (!query.exec()) {
        m_dernierErreur = query.lastError().text();
        return false;
    }
    return true;
}