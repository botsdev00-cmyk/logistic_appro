#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include <QMessageBox>
#include <QDebug>

#include "ui/main_window/FenetreMain.h"
#include "ui/dialogs/BoiteDialogConnexion.h"
#include "business/services/ServiceAuthentification.h"
#include "data/database/ConnexionBaseDonnees.h"
#include "core/entities/Utilisateur.h"

class SplashScreen
{
public:
    static void show()
    {
        QPixmap pixmap(":/ images/splash.png");
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
    
    // Configuration de l'application
    app.setApplicationName("SEMULIKI ERP");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("SEMULIKI");
    
    // Style par défaut
    app.setStyle("Fusion");
    
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "  SEMULIKI ERP - Système de Gestion Logistique";
    qDebug() << "  Version: 1.0.0";
    qDebug() << "  Date: 2026-04-10";
    qDebug() << "═══════════════════════════════════════════════════════";
    
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
            return 0;
        }
        
        qDebug() << "[OK] Utilisateur connecté";
        
        // 3. Créer et afficher la fenêtre principale
        qDebug() << "\n[INFO] Création de la fenêtre principale...";
        Utilisateur utilisateurConnecte;
        FenetreMain window(utilisateurConnecte);
        window.show();
        
        qDebug() << "[OK] Application démarrée avec succès\n";
        
        // 4. Lancer la boucle d'événements
        return app.exec();
        
    } catch (const std::exception& e) {
        qCritical() << "[ERREUR FATALE]" << e.what();
        QMessageBox::critical(nullptr, "Erreur fatale",
            QString("Une erreur fatale s'est produite:\n%1").arg(e.what()));
        return 1;
    } catch (...) {
        qCritical() << "[ERREUR INCONNUE] Une erreur inconnue s'est produite";
        QMessageBox::critical(nullptr, "Erreur inconnue",
            "Une erreur inconnue s'est produite.");
        return 1;
    }
}