//
// Created by Mario Marchand on 16-12-29.
//

#include "DonneesGTFS.h"

using namespace std;


//! \brief ajoute les lignes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les lignes
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterLignes(const std::string &p_nomFichier)
{
    ifstream fichierRoutes(p_nomFichier);
    if (fichierRoutes.bad()) {throw logic_error("fichier introuvable");} //Vérifie si le fichier existe
    string lignesDuFichier;

    while (getline( fichierRoutes, lignesDuFichier))
    {
        if (lignesDuFichier[0] != 'r') // pour éviter la première ligne du fichier (à changer ? TODO)
        {
            vector<string> ligneVec = string_to_vector(lignesDuFichier, ',');
            string p_numero = ligneVec[2];
            p_numero.erase(remove(p_numero.begin(), p_numero.end(), '\"'), p_numero.end());
            string p_description = ligneVec[4];
            p_description.erase(remove(p_description.begin(), p_description.end(), '\"'), p_description.end());

            Ligne newLigne(stoi(ligneVec[0]), p_numero, p_description, Ligne::couleurToCategorie(ligneVec[7])); // est-ce que je serais mieux d'uniformiser pour que ça soit toutes des var ? TODO
            this->m_lignes[newLigne.getId()] = newLigne;
            this->m_lignes_par_numero.insert(make_pair(newLigne.getNumero(), newLigne));
            ligneVec.clear(); //supprime le contenu du vecteur après utilisation
        }
    }

}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{
    ifstream fichierStations(p_nomFichier);
    if (fichierStations.bad()) {throw logic_error("fichier introuvable");} //Vérifie si le fichier existe
    string lignesDuFichier;

    while (getline( fichierStations, lignesDuFichier))
    {
        if (lignesDuFichier[0] != 's') // pour éviter la première ligne du fichier (à changer ? TODO)
        {
            vector<string> arretVec = string_to_vector(lignesDuFichier, ',');
            string p_nom = arretVec[1];
            p_nom.erase(remove(p_nom.begin(), p_nom.end(), '\"'), p_nom.end());
            string p_description = arretVec[2];
            p_description.erase(remove(p_description.begin(), p_description.end(), '\"'), p_description.end());
            Coordonnees coordStation(stod(arretVec[3]), stod(arretVec[4]));
            Station newStation(stoi(arretVec[0]), p_nom, p_description, coordStation);
            this->m_stations[stoi(arretVec[0])] = newStation;
            arretVec.clear();
        }
    }
}

//! \brief ajoute les transferts dans l'objet GTFS
//! \breif Cette méthode doit âtre utilisée uniquement après que tous les arrêts ont été ajoutés
//! \brief les transferts (entre stations) ajoutés sont uniquement ceux pour lesquelles les stations sont prensentes dans l'objet GTFS
//! \brief les transferts sont ajoutés dans m_transferts
//! \brief les from_station_id des stations de transfert sont ajoutés dans m_stationsDeTransfert
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
//! \throws logic_error si tous les arrets de la date et de l'intervalle n'ont pas été ajoutés
void DonneesGTFS::ajouterTransferts(const std::string &p_nomFichier)
{
    //écrire votre code ici
}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{

    ifstream fichierServices(p_nomFichier);
    if (fichierServices.bad()) {throw logic_error("fichier introuvable");} //Vérifie si le fichier existe
    string lignesDuFichier;

    while (getline(fichierServices, lignesDuFichier))
    {
        if (lignesDuFichier[0] != 's') // pour éviter la première ligne du fichier (à changer ? TODO)
        {
            vector<string> servicesVec = string_to_vector(lignesDuFichier, ','); // est-ce necessaire de convertir en vec? TODO
            Date dateDeService(stoi(servicesVec[1].substr(0,4)),
                    stoi(servicesVec[1].substr(4,2)),
                    stoi(servicesVec[1].substr(6,2)));

            if (servicesVec[2] == "1" && this->m_date == dateDeService)
            {
                this->m_services.insert(servicesVec[0]);
            }
            servicesVec.clear();
        }
    }


}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{

//écrire votre code ici

}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier)
{

//écrire votre code ici

}



