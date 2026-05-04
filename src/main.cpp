#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include <QMessageBox>
#include <QDebug>
#include "utils/globals/globals.h"
#include "ui/main_window/FenetreMain.h"
#include "ui/dialogs/BoiteDialogConnexion.h"
#include "business/services/ServiceAuthentification.h"
#include "data/database/ConnexionBaseDonnees.h"
#include "core/entities/Utilisateur.h"

#include "business/managers/GestionnaireRepartition.h"
#include "business/managers/GestionnaireSales.h"
#include "business/managers/GestionnaireCredit.h"
#include "business/managers/GestionnaireStock.h"
#include "core/entities/ArticleRepartition.h"

// Inclure les repositories stock !
#include "data/repositories/RepositoryRetourStock.h"
#include "data/repositories/RepositoryEntreeStock.h"
#include "data/repositories/RepositoryStockSoldes.h"
#include "data/repositories/RepositoryStockMouvements.h"

class SplashScreen
{
public:
    static void show()
    {
        QPixmap pixmap(":/images/splash.png");
        QSplashScreen splash(pixmap);
        splash.show();
        splash.showMessage("Chargement de SEMULIKI ERP...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        QApplication::processEvents();
        QThread::msleep(2000);
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("SEMULIKI ERP");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("SEMULIKI");

    app.setStyle("Fusion");

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "  SEMULIKI ERP - Système de Gestion Logistique";
    qDebug() << "  Version: 1.0.0";
    qDebug() << "  Date: 2026-04-10";
    qDebug() << "═══════════════════════════════════════════════════════";

    // ----------------------- INITIALISATION DES MANAGERS GLOBAUX ----------------------
    g_repartitionMgr = new GestionnaireRepartition();
    g_venteMgr = new GestionnaireSales();
    g_creditMgr = new GestionnaireCredit();
    g_stockMgr = new GestionnaireStock();
    art = new ArticleRepartition();

    // ----------------------- INITIALISE LES REPOSITORIES DU STOCK ----------------------
    auto repoRetours   = new RepositoryRetourStock();
    auto repoEntrees   = new RepositoryEntreeStock();
    auto repoSoldes    = new RepositoryStockSoldes();
    auto repoMouvements= new RepositoryStockMouvements();

    // ----------------------- INJECTION DANS LE GESTIONNAIRE STOCK GLOBAL ----------------
    g_stockMgr->setRepositoryRetourStock(repoRetours);
    g_stockMgr->setRepositoryEntreeStock(repoEntrees);
    g_stockMgr->setRepositoryStockSoldes(repoSoldes);
    g_stockMgr->setRepositoryStockMouvements(repoMouvements);

    int result = 1;

    try {
        // 1. Initialiser la connexion à la base de données
        qDebug() << "\n[INFO] Connexion à la base de données...";

        auto& db = ConnexionBaseDonnees::getInstance();
        if (!db.initialiser("localhost", "5432", "semuliki", "postgres", "postgresql")) {
            qCritical() << "[ERREUR] Impossible de se connecter à la base de données";
            QMessageBox::critical(nullptr, "Erreur base de données",
                "Impossible de se connecter à la base de données.\n"
                "Vérifiez votre configuration PostgreSQL.");
            return 1;
        }

        qDebug() << "[OK] Base de données connectée";

        // 2. Afficher l'écran de connexion
        qDebug() << "\n[INFO] Affichage de la fenêtre de connexion...";
        BoiteDialogConnexion dlgConnexion;

        if (dlgConnexion.exec() != QDialog::Accepted) {
            qDebug() << "[INFO] Connexion annulée par l'utilisateur";
            goto cleanup;
        }

        qDebug() << "[OK] Utilisateur connecté";

        // 3. Récupérer l'utilisateur connecté
        qDebug() << "\n[INFO] Récupération des données utilisateur...";
        Utilisateur utilisateurConnecte = dlgConnexion.getUtilisateurConnecte();

        if (utilisateurConnecte.getUtilisateurId().isNull()) {
            qCritical() << "[ERREUR] L'ID utilisateur n'a pas pu être récupéré";
            QMessageBox::critical(nullptr, "Erreur authentification",
                "Impossible de récupérer les données utilisateur.\n"
                "Veuillez vous reconnecter.");
            goto cleanup;
        }

        qDebug() << "[OK] Utilisateur:" << utilisateurConnecte.getNomUtilisateur();
        qDebug() << "[OK] UUID:" << utilisateurConnecte.getUtilisateurId().toString();

        g_utilisateurId = utilisateurConnecte.getUtilisateurId();

        // 4. Créer et afficher la fenêtre principale
        qDebug() << "\n[INFO] Création de la fenêtre principale...";
        FenetreMain window(utilisateurConnecte); // note: on passe l'utilisateur, le gestionnaire est global
        window.show();

        qDebug() << "[OK] Application démarrée avec succès\n";

        result = app.exec();

    } catch (const std::exception& e) {
        qCritical() << "[ERREUR FATALE]" << e.what();
        QMessageBox::critical(nullptr, "Erreur fatale",
            QString("Une erreur fatale s'est produite:\n%1").arg(e.what()));
        result = 1;
    } catch (...) {
        qCritical() << "[ERREUR INCONNUE] Une erreur inconnue s'est produite";
        QMessageBox::critical(nullptr, "Erreur inconnue",
            "Une erreur inconnue s'est produite.");
        result = 1;
    }

cleanup:
    // Nettoyage mémoire managers et pointeur d'article si besoin
    delete g_repartitionMgr; g_repartitionMgr = nullptr;
    delete g_venteMgr;       g_venteMgr = nullptr;
    delete g_creditMgr;      g_creditMgr = nullptr;
    delete g_stockMgr;       g_stockMgr = nullptr;
    delete art;              art = nullptr;
    delete repoRetours;
    delete repoEntrees;
    delete repoSoldes;
    delete repoMouvements;

    return result;
}