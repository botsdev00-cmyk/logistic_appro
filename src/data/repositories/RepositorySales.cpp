#include "RepositorySales.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositorySales::RepositorySales()
{
}

bool RepositorySales::create(const Vente& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO ventes "
                  "(vente_id, repartition_id, produit_id, client_id, quantite, type_vente, prix_unitaire, notes, cree_par) "
                  "VALUES (:id, :rep_id, :prod_id, :client_id, :qty, :type, :prix, :notes, :cree_par)");

    query.addBindValue(entity.getVenteId().toString());
    query.addBindValue(entity.getRepartitionId().toString());
    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getClientId().toString());
    query.addBindValue(entity.getQuantite());
    query.addBindValue(Vente::typeVenteToString(entity.getTypeVente()));
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNotes());
    query.addBindValue(entity.getCreePar().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création vente : " + query.lastError().text();
        return false;
    }

    return true;
}

Vente RepositorySales::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM ventes WHERE vente_id = :id");
    query.addBindValue(id.toString());

    Vente vente;
    if (query.exec() && query.next()) {
        vente.setVenteId(QUuid(query.value("vente_id").toString()));
        vente.setRepartitionId(QUuid(query.value("repartition_id").toString()));
        vente.setProduitId(QUuid(query.value("produit_id").toString()));
        vente.setClientId(QUuid(query.value("client_id").toString()));
        vente.setQuantite(query.value("quantite").toInt());
        vente.setTypeVente(Vente::stringToTypeVente(query.value("type_vente").toString()));
        vente.setPrixUnitaire(query.value("prix_unitaire").toDouble());
        vente.setNotes(query.value("notes").toString());
        vente.setDateVente(query.value("date_vente").toDateTime());
    } else {
        m_dernierErreur = "Vente non trouvée";
    }

    return vente;
}

QList<Vente> RepositorySales::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Vente> ventes;

    query.prepare("SELECT * FROM ventes ORDER BY date_vente DESC");

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.setVenteId(QUuid(query.value("vente_id").toString()));
            vente.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            vente.setClientId(QUuid(query.value("client_id").toString()));
            vente.setQuantite(query.value("quantite").toInt());
            vente.setTypeVente(Vente::stringToTypeVente(query.value("type_vente").toString()));
            ventes.append(vente);
        }
    }

    return ventes;
}

bool RepositorySales::update(const Vente& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE ventes SET "
                  "quantite = :qty, type_vente = :type, prix_unitaire = :prix, notes = :notes "
                  "WHERE vente_id = :id");

    query.addBindValue(entity.getQuantite());
    query.addBindValue(Vente::typeVenteToString(entity.getTypeVente()));
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNotes());
    query.addBindValue(entity.getVenteId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour vente : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositorySales::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("DELETE FROM ventes WHERE vente_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression vente : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Vente> RepositorySales::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Vente> ventes;

    query.prepare("SELECT v.* FROM ventes v "
                  "JOIN clients c ON v.client_id = c.client_id "
                  "WHERE c.nom ILIKE :criterion ORDER BY v.date_vente DESC");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.setVenteId(QUuid(query.value("vente_id").toString()));
            ventes.append(vente);
        }
    }

    return ventes;
}

bool RepositorySales::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM ventes WHERE vente_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

QList<Vente> RepositorySales::getByRepartition(const QUuid& repartitionId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Vente> ventes;

    query.prepare("SELECT * FROM ventes WHERE repartition_id = :rep_id ORDER BY date_vente");
    query.addBindValue(repartitionId.toString());

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.setVenteId(QUuid(query.value("vente_id").toString()));
            vente.setQuantite(query.value("quantite").toInt());
            ventes.append(vente);
        }
    }

    return ventes;
}

QList<Vente> RepositorySales::getByClient(const QUuid& clientId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Vente> ventes;

    query.prepare("SELECT * FROM ventes WHERE client_id = :client_id ORDER BY date_vente DESC");
    query.addBindValue(clientId.toString());

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.setVenteId(QUuid(query.value("vente_id").toString()));
            ventes.append(vente);
        }
    }

    return ventes;
}

double RepositorySales::getTotalVentes(const QUuid& repartitionId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT SUM(montant_total) as total FROM ventes WHERE repartition_id = :rep_id");
    query.addBindValue(repartitionId.toString());

    if (query.exec() && query.next()) {
        return query.value("total").toDouble();
    }

    return 0.0;
}

double RepositorySales::getTotalCash(const QUuid& repartitionId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT SUM(montant_total) as total FROM ventes WHERE repartition_id = :rep_id AND type_vente = 'cash'");
    query.addBindValue(repartitionId.toString());

    if (query.exec() && query.next()) {
        return query.value("total").toDouble();
    }

    return 0.0;
}