#ifndef BOITEDIALOGENEXION_H
#define BOITEDIALOGENEXION_H

#include <QDialog>
#include <memory>

class QLineEdit;
class QPushButton;
class QLabel;
class ServiceAuthentification;

class BoiteDialogConnexion : public QDialog
{
    Q_OBJECT

public:
    BoiteDialogConnexion(QWidget* parent = nullptr);
    ~BoiteDialogConnexion();

private slots:
    void seConnecter();
    void afficherMotDePasse();
    void reinitialiser();

private:
    void creerWidgets();
    void initialiserConnexions();
    void configurerStyleSheet();

    std::unique_ptr<QLineEdit> m_champUtilisateur;
    std::unique_ptr<QLineEdit> m_champMotDePasse;
    std::unique_ptr<QPushButton> m_boutonConnecter;
    std::unique_ptr<QPushButton> m_boutonAnnuler;
    std::unique_ptr<QPushButton> m_boutonAfficherMotDePasse;
    std::unique_ptr<QLabel> m_labelErreur;
    
    std::unique_ptr<ServiceAuthentification> m_authService;
};

#endif // BOITEDIALOGENEXION_H