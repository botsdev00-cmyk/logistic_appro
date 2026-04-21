#include "RepositoryEquipe.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryEquipe::RepositoryEquipe()
{
}

bool RepositoryEquipe::create(const Equipe& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO equipes (equipe_id, nom, nom_chef) VALUES (:id, :nom, :nom_chef)");
    query.addBindValue(entity.getEquipeId().toString());
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getNomChef());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création équipe : " + query.lastError().text();
        return false;
    }

    return true;
}

Equipe RepositoryEquipe::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM equipes WHERE equipe_id = :id");
    query.addBindValue(id.toString());

    Equipe equipe;
    if (query.exec() && query.next()) {
        equipe.setEquipeId(QUuid(query.value("equipe_id").toString()));
        equipe.setNom(query.value("nom").toString());
        equipe.setNomChef(query.value("nom_chef").toString());
        // Ajoute ici d'autres champs si tu en ajoutes à l'avenir
    } else {
        m_dernierErreur = "Équipe non trouvée";
    }

    return equipe;
}

QList<Equipe> RepositoryEquipe::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare("SELECT * FROM equipes ORDER BY nom");

    if (query.exec()) {
        while (query.next()) {
            Equipe equipe;
            equipe.setEquipeId(QUuid(query.value("equipe_id").toString()));
            equipe.setNom(query.value("nom").toString());
            equipe.setNomChef(query.value("nom_chef").toString());
            equipes.append(equipe);
        }
    }

    return equipes;
}

bool RepositoryEquipe::update(const Equipe& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE equipes SET nom = :nom, nom_chef = :nom_chef WHERE equipe_id = :id");
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getNomChef());
    query.addBindValue(entity.getEquipeId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour équipe : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryEquipe::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    // Suppression physique du membre - car pas de colonne est_actif (si tu veux un soft delete, ajoute la colonne !)
    query.prepare("DELETE FROM equipes WHERE equipe_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression équipe : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Equipe> RepositoryEquipe::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare("SELECT * FROM equipes WHERE nom ILIKE :criterion ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Equipe equipe;
            equipe.setEquipeId(QUuid(query.value("equipe_id").toString()));
            equipe.setNom(query.value("nom").toString());
            equipe.setNomChef(query.value("nom_chef").toString());
            equipes.append(equipe);
        }
    }

    return equipes;
}

bool RepositoryEquipe::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM equipes WHERE equipe_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}