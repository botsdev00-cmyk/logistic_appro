#ifndef GENERATEURPDF_H
#define GENERATEURPDF_H

#include <QString>
#include <QDate>

/**
 * @class GenerateurPDF
 * @brief Classe utilitaire pour la génération de PDF
 * 
 * Fournit des méthodes pour générer des fichiers PDF à partir
 * des données de l'application (factures, rapports, etc.).
 */
class GenerateurPDF
{
public:
    /**
     * @brief Initialise le générateur de PDF
     * @param dossierSortie Dossier où sauvegarder les PDF
     * @return true si succès, false sinon
     */
    static bool initialiser(const QString& dossierSortie = "./pdf");

    /**
     * @brief Génère une facture en PDF
     * @param numeroFacture Numéro de la facture
     * @param cheminSortie Chemin où sauvegarder le PDF
     * @return true si succès, false sinon
     */
    static bool genererFacture(const QString& numeroFacture, QString& cheminSortie);

    /**
     * @brief Génère un reçu en PDF
     * @param numeroRecu Numéro du reçu
     * @param cheminSortie Chemin où sauvegarder le PDF
     * @return true si succès, false sinon
     */
    static bool genererRecu(const QString& numeroRecu, QString& cheminSortie);

    /**
     * @brief Génère un rapport en PDF
     * @param titre Titre du rapport
     * @param contenu Contenu du rapport
     * @param cheminSortie Chemin où sauvegarder le PDF
     * @return true si succès, false sinon
     */
    static bool genererRapport(const QString& titre, const QString& contenu, QString& cheminSortie);

    /**
     * @brief Obtient le dernier message d'erreur
     * @return Message d'erreur
     */
    static QString getLastError();

private:
    /**
     * @brief Constructeur privé (classe utilitaire statique)
     */
    GenerateurPDF() = default;

    /**
     * @brief Génère un nom de fichier unique
     * @param prefixe Préfixe du fichier
     * @return Chemin complet du fichier PDF
     */
    static QString genererNomFichier(const QString& prefixe);

    // Dossier de sortie
    static QString m_dossierSortie;
    static QString m_lastError;
    static bool m_initialise;
};

#endif // GENERATEURPDF_H