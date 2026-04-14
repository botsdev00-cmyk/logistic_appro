#include "MigrationBaseDonnees.h"
#include "ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

bool MigrationBaseDonnees::executerMigrationsInitiales()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    if (!bd.estConnecte()) {
        qDebug() << "Erreur : Base de données non connectée";
        return false;
    }

    if (verifierSchemaExiste()) {
        qDebug() << "Schéma déjà créé";
        return true;
    }

    qDebug() << "Exécution des migrations initiales...";
    // Tu adaptes ici selon ton arborescence de fichier SQL !
    return executerMigration("scripts/structure.sql");
}

bool MigrationBaseDonnees::executerMigration(const QString& cheminSQL)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QString contenu = obtenirContenuMigration(cheminSQL);

    if (contenu.isEmpty()) {
        qDebug() << "Erreur : Migration vide -" << cheminSQL;
        return false;
    }

    // On coupe les instructions si besoin, ici naïvement par ';' (attention : simple)
    QStringList instructions = contenu.split(';', Qt::SkipEmptyParts);
    QSqlQuery query(bd.getDatabase());
    for (const QString& instr : instructions) {
        QString sql = instr.trimmed();
        if (sql.isEmpty()) continue;
        if (!query.exec(sql)) {
            qDebug() << "Erreur migration sur :" << sql.left(120) << "..." << query.lastError().text();
            return false;
        }
    }

    qDebug() << "Migration réussie :" << cheminSQL;
    return true;
}

bool MigrationBaseDonnees::verifierSchemaExiste()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    // On vérifie la présence de la table "utilisateurs"
    if (!query.exec("SELECT 1 FROM utilisateurs LIMIT 1")) {
        qDebug() << "[DEBUG] Table 'utilisateurs' absente, schema doit être créé.";
        return false;
    }
    return true;
}

QString MigrationBaseDonnees::obtenirContenuMigration(const QString& cheminSQL)
{
    QFile fichier(cheminSQL);
    if (!fichier.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Impossible d'ouvrir le fichier de migration:" << cheminSQL;
        return "";
    }
    QString contenu = fichier.readAll();
    fichier.close();
    return contenu;
}