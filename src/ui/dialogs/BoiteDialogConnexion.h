#ifndef BOITEDIALOGCONNEXION_H
#define BOITEDIALOGCONNEXION_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "../../core/entities/Utilisateur.h"

class ServiceAuthentification;

class BoiteDialogConnexion : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogConnexion(QWidget* parent = nullptr);
    ~BoiteDialogConnexion();
    
    Utilisateur getUtilisateurConnecte() const { return m_utilisateurConnecte; }

private slots:
    void seConnecter();
    void afficherMotDePasse();

private:
    void creerWidgets();
    void initialiserConnexions();
    void configurerStyleSheet();
    void reinitialiser();
    
    std::unique_ptr<ServiceAuthentification> m_authService;
    std::unique_ptr<QLineEdit> m_champUtilisateur;
    std::unique_ptr<QLineEdit> m_champMotDePasse;
    std::unique_ptr<QPushButton> m_boutonConnecter;
    std::unique_ptr<QPushButton> m_boutonAnnuler;
    std::unique_ptr<QPushButton> m_boutonAfficherMotDePasse;
    std::unique_ptr<QLabel> m_labelErreur;
    
    Utilisateur m_utilisateurConnecte;
};

#endif // BOITEDIALOGCONNEXION_H