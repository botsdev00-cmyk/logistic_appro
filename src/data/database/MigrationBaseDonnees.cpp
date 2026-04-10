#include "MigrationBaseDonnees.h"
#include "ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool MigrationBaseDonnees::executerMigrationsInitiales()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    
    if (!bd.estConnecte()) {
        qDebug() << "Erreur : Base de données non connectée";
        return false;
    }

    // Vérifier si le schéma existe déjà
    if (verifierSchemaExiste()) {
        qDebug() << "Schéma déjà créé";
        return true;
    }

    qDebug() << "Exécution des migrations initiales...";
    return executerMigration("001_initial_schema");
}

bool MigrationBaseDonnees::executerMigration(const QString& nomMigration)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QString contenu = obtenirContenuMigration(nomMigration);

    if (contenu.isEmpty()) {
        qDebug() << "Erreur : Migration vide -" << nomMigration;
        return false;
    }

    QSqlQuery query(bd.getDatabase());
    if (!query.exec(contenu)) {
        qDebug() << "Erreur de migration :" << query.lastError().text();
        return false;
    }

    qDebug() << "Migration réussie :" << nomMigration;
    return true;
}

bool MigrationBaseDonnees::verifierSchemaExiste()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    return query.exec("SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_name = 'utilisateurs')");
}

QString MigrationBaseDonnees::obtenirContenuMigration(const QString& nomMigration)
{
    // À implémenter : charger le contenu SQL du fichier de migration
    // Pour l'instant, retourner une chaîne vide
    return "";
}