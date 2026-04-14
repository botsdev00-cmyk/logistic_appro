#include "RepositoryUtilisateur.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

RepositoryUtilisateur::RepositoryUtilisateur()
{
}

bool RepositoryUtilisateur::create(const Utilisateur& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO utilisateur "
                  "(utilisateur_id, nom_utilisateur, email, hash_mot_passe, nom_complet, role_id, est_actif) "
                  "VALUES (:id, :nom_utilisateur, :email, :hash, :nom_complet, "
                  "(SELECT role_id FROM role WHERE libelle = :role), :actif)");

    query.addBindValue(entity.getUtilisateurId().toString());
    query.addBindValue(entity.getNomUtilisateur());
    query.addBindValue(entity.getEmail());
    query.addBindValue(entity.getHashMotPasse());
    query.addBindValue(entity.getNomComplet());
    query.addBindValue(Utilisateur::roleToString(entity.getRole()));
    query.addBindValue(entity.estActif());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création utilisateur : " + query.lastError().text();
        qDebug() << m_dernierErreur;
        return false;
    }

    return true;
}

Utilisateur RepositoryUtilisateur::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT u.*, r.libelle as role_nom "
                  "FROM utilisateurs u "
                  "LEFT JOIN role r ON u.role_id = r.role_id "
                  "WHERE u.utilisateur_id = :id");
    query.addBindValue(id.toString());

    Utilisateur utilisateur;
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        int idx = rec.indexOf("hash_mot_passe");
        QString hash;
        if (idx != -1) {
            hash = query.value(idx).toString();
            qDebug() << "[getById] hash_mot_passe from SQL =" << hash;
        } else {
            qWarning() << "[getById] PAS de champ 'hash_mot_passe' dans ce SELECT !";
        }
        utilisateur.setUtilisateurId(QUuid(query.value("utilisateur_id").toString()));
        utilisateur.setNomUtilisateur(query.value("nom_utilisateur").toString());
        utilisateur.setEmail(query.value("email").toString());
        utilisateur.setHashMotPasse(hash); // Même si vide, pour le debug
        utilisateur.setNomComplet(query.value("nom_complet").toString());
        utilisateur.setRole(Utilisateur::stringToRole(query.value("role_nom").toString()));
        utilisateur.setEstActif(query.value("est_actif").toBool());
        utilisateur.setDateCreation(query.value("date_creation").toDateTime());
        utilisateur.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
    } else {
        m_dernierErreur = "Utilisateur non trouvé";
    }

    return utilisateur;
}

QList<Utilisateur> RepositoryUtilisateur::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Utilisateur> utilisateurs;

    query.prepare("SELECT u.*, r.libelle as role_nom "
                  "FROM utilisateurs u "
                  "LEFT JOIN role r ON u.role_id = r.role_id "
                  "WHERE u.est_actif = true ORDER BY u.nom_complet");

    if (query.exec()) {
        QSqlRecord rec = query.record();
        int idx = rec.indexOf("hash_mot_passe");
        while (query.next()) {
            Utilisateur utilisateur;
            QString hash;
            if (idx != -1) {
                hash = query.value(idx).toString();
                qDebug() << "[getAll] hash_mot_passe from SQL =" << hash;
            } else {
                qWarning() << "[getAll] PAS de champ 'hash_mot_passe' !";
            }
            utilisateur.setUtilisateurId(QUuid(query.value("utilisateur_id").toString()));
            utilisateur.setNomUtilisateur(query.value("nom_utilisateur").toString());
            utilisateur.setEmail(query.value("email").toString());
            utilisateur.setHashMotPasse(hash); // Important
            utilisateur.setNomComplet(query.value("nom_complet").toString());
            utilisateur.setRole(Utilisateur::stringToRole(query.value("role_nom").toString()));
            utilisateur.setEstActif(query.value("est_actif").toBool());
            utilisateurs.append(utilisateur);
        }
    } else {
        m_dernierErreur = "Erreur lecture utilisateurs : " + query.lastError().text();
    }

    return utilisateurs;
}

bool RepositoryUtilisateur::update(const Utilisateur& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE utilisateur SET "
                  "nom_utilisateur = :nom_utilisateur, email = :email, "
                  "hash_mot_passe = :hash, nom_complet = :nom_complet, "
                  "role_id = (SELECT role_id FROM role WHERE libelle = :role), "
                  "est_actif = :actif "
                  "WHERE utilisateur_id = :id");

    query.addBindValue(entity.getNomUtilisateur());
    query.addBindValue(entity.getEmail());
    query.addBindValue(entity.getHashMotPasse());
    query.addBindValue(entity.getNomComplet());
    query.addBindValue(Utilisateur::roleToString(entity.getRole()));
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getUtilisateurId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour utilisateur : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryUtilisateur::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE utilisateur SET est_actif = false WHERE utilisateur_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression utilisateur : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Utilisateur> RepositoryUtilisateur::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Utilisateur> utilisateurs;

    query.prepare("SELECT u.*, r.libelle as role_nom "
                  "FROM utilisateurs u "
                  "LEFT JOIN role r ON u.role_id = r.role_id "
                  "WHERE u.nom_complet ILIKE :criterion OR u.nom_utilisateur ILIKE :criterion "
                  "ORDER BY u.nom_complet");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        QSqlRecord rec = query.record();
        int idx = rec.indexOf("hash_mot_passe");
        while (query.next()) {
            Utilisateur utilisateur;
            QString hash;
            if (idx != -1) {
                hash = query.value(idx).toString();
                qDebug() << "[search] hash_mot_passe from SQL =" << hash;
            } else {
                qWarning() << "[search] PAS de champ 'hash_mot_passe' !";
            }
            utilisateur.setUtilisateurId(QUuid(query.value("utilisateur_id").toString()));
            utilisateur.setNomUtilisateur(query.value("nom_utilisateur").toString());
            utilisateur.setEmail(query.value("email").toString());
            utilisateur.setHashMotPasse(hash); // Important
            utilisateur.setNomComplet(query.value("nom_complet").toString());
            utilisateur.setRole(Utilisateur::stringToRole(query.value("role_nom").toString()));
            utilisateurs.append(utilisateur);
        }
    }

    return utilisateurs;
}

bool RepositoryUtilisateur::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM utilisateurs WHERE utilisateur_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

Utilisateur RepositoryUtilisateur::getByUsername(const QString& username)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT u.*, r.libelle as role_nom "
                  "FROM utilisateurs u "
                  "LEFT JOIN role r ON u.role_id = r.role_id "
                  "WHERE u.nom_utilisateur = :username");
    query.addBindValue(username);

    Utilisateur utilisateur;
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        int idx = rec.indexOf("hash_mot_passe");
        QString hash;
        if (idx != -1) {
            hash = query.value(idx).toString();
            qDebug() << "[getByUsername] hash_mot_passe from SQL =" << hash;
        } else {
            qWarning() << "[getByUsername] PAS de champ 'hash_mot_passe' dans ce SELECT !";
        }
        utilisateur.setUtilisateurId(QUuid(query.value("utilisateur_id").toString()));
        utilisateur.setNomUtilisateur(query.value("nom_utilisateur").toString());
        utilisateur.setEmail(query.value("email").toString());
        utilisateur.setHashMotPasse(hash);
        utilisateur.setNomComplet(query.value("nom_complet").toString());
        utilisateur.setRole(Utilisateur::stringToRole(query.value("role_nom").toString()));
    }

    return utilisateur;
}

Utilisateur RepositoryUtilisateur::getByEmail(const QString& email)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT u.*, r.libelle as role_nom "
                  "FROM utilisateurs u "
                  "LEFT JOIN role r ON u.role_id = r.role_id "
                  "WHERE u.email = :email");
    query.addBindValue(email);

    Utilisateur utilisateur;
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        int idx = rec.indexOf("hash_mot_passe");
        QString hash;
        if (idx != -1) {
            hash = query.value(idx).toString();
            qDebug() << "[getByEmail] hash_mot_passe from SQL =" << hash;
        } else {
            qWarning() << "[getByEmail] PAS de champ 'hash_mot_passe' dans ce SELECT !";
        }
        utilisateur.setUtilisateurId(QUuid(query.value("utilisateur_id").toString()));
        utilisateur.setEmail(query.value("email").toString());
        utilisateur.setHashMotPasse(hash); // Ajout important
        utilisateur.setNomComplet(query.value("nom_complet").toString());
        utilisateur.setRole(Utilisateur::stringToRole(query.value("role_nom").toString()));
    }

    return utilisateur;
}