#ifndef GENERATEURRAPPORT_H
#define GENERATEURRAPPORT_H

#include <QString>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QDate>

/**
 * @class GenerateurRapport
 * @brief Classe utilitaire pour la génération de rapports
 * 
 * Fournit des méthodes pour générer des rapports au format texte,
 * HTML et CSV à partir des données de l'application.
 */
class GenerateurRapport
{
public:
    /**
     * @enum FormatRapport
     * @brief Énumération des formats de sortie
     */
    enum class FormatRapport {
        Texte,  ///< Format texte plain
        HTML,   ///< Format HTML
        CSV,    ///< Format CSV (Excel)
        JSON    ///< Format JSON
    };

    /**
     * @brief Initialise le générateur de rapports
     * @param dossierSortie Dossier où sauvegarder les rapports
     * @return true si succès, false sinon
     */
    static bool initialiser(const QString& dossierSortie = "./rapports");

    /**
     * @brief Génère un rapport de ventes
     * @param dateDebut Date de début
     * @param dateFin Date de fin
     * @param format Format de sortie
     * @return Chemin du fichier généré ou vide si erreur
     */
    static QString genererRapportVentes(
        const QDate& dateDebut,
        const QDate& dateFin,
        FormatRapport format = FormatRapport::HTML
    );

    /**
     * @brief Génère un rapport de stock
     * @param format Format de sortie
     * @return Chemin du fichier généré ou vide si erreur
     */
    static QString genererRapportStock(FormatRapport format = FormatRapport::HTML);

    /**
     * @brief Génère un rapport de crédits
     * @param dateDebut Date de début
     * @param dateFin Date de fin
     * @param format Format de sortie
     * @return Chemin du fichier généré ou vide si erreur
     */
    static QString genererRapportCredits(
        const QDate& dateDebut,
        const QDate& dateFin,
        FormatRapport format = FormatRapport::HTML
    );

    /**
     * @brief Génère un rapport de caisse
     * @param dateDebut Date de début
     * @param dateFin Date de fin
     * @param format Format de sortie
     * @return Chemin du fichier généré ou vide si erreur
     */
    static QString genererRapportCaisse(
        const QDate& dateDebut,
        const QDate& dateFin,
        FormatRapport format = FormatRapport::HTML
    );

    /**
     * @brief Génère un rapport de performance par équipe
     * @param dateDebut Date de début
     * @param dateFin Date de fin
     * @param format Format de sortie
     * @return Chemin du fichier généré ou vide si erreur
     */
    static QString genererRapportPerformanceEquipes(
        const QDate& dateDebut,
        const QDate& dateFin,
        FormatRapport format = FormatRapport::HTML
    );

    /**
     * @brief Génère un rapport personnalisé
     * @param titre Titre du rapport
     * @param donnees Données du rapport (map clé-valeur)
     * @param format Format de sortie
     * @return Chemin du fichier généré ou vide si erreur
     */
    static QString genererRapportPersonnalise(
        const QString& titre,
        const QMap<QString, QVariant>& donnees,
        FormatRapport format = FormatRapport::HTML
    );

    /**
     * @brief Obtient le dernier message d'erreur
     * @return Message d'erreur
     */
    static QString getLastError();

    /**
     * @brief Ouvre un rapport dans le navigateur par défaut
     * @param cheminFichier Chemin du fichier à ouvrir
     * @return true si succès, false sinon
     */
    static bool ouvrirRapport(const QString& cheminFichier);

private:
    /**
     * @brief Constructeur privé (classe utilitaire statique)
     */
    GenerateurRapport() = default;

    /**
     * @brief Génère un nom de fichier unique pour le rapport
     * @param prefixe Préfixe du fichier
     * @param extension Extension du fichier
     * @return Chemin complet du fichier
     */
    static QString genererNomFichier(const QString& prefixe, const QString& extension);

    /**
     * @brief Convertit un format enum en extension de fichier
     * @param format Format du rapport
     * @return Extension de fichier (.html, .csv, etc.)
     */
    static QString formatToExtension(FormatRapport format);

    // Dossier de sortie
    static QString m_dossierSortie;
    static QString m_lastError;
    static bool m_initialise;
};

#endif // GENERATEURRAPPORT_H