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

            unsigned int id = stoi(ligneVec[0]);
            string numero = ligneVec[2];
            numero.erase(remove(numero.begin(), numero.end(), '\"'), numero.end());
            string p_description = ligneVec[4];
            p_description.erase(remove(p_description.begin(), p_description.end(), '\"'), p_description.end());
            CategorieBus categorie = Ligne::couleurToCategorie(ligneVec[7]);

            Ligne nouvelleLigne(id, numero, p_description, categorie);
            m_lignes[id] = nouvelleLigne;
            m_lignes_par_numero.insert(make_pair(numero, nouvelleLigne));

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
        if (lignesDuFichier[0] != 's')
        {
            vector<string> arretVec = string_to_vector(lignesDuFichier, ',');

            string p_nom = arretVec[1];
            p_nom.erase(remove(p_nom.begin(), p_nom.end(), '\"'), p_nom.end());
            string p_description = arretVec[2];
            p_description.erase(remove(p_description.begin(), p_description.end(), '\"'), p_description.end());
            Coordonnees coordStation(stod(arretVec[3]), stod(arretVec[4]));

            Station newStation(stoi(arretVec[0]), p_nom, p_description, coordStation);
            m_stations[stoi(arretVec[0])] = newStation;
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
    ifstream fichierTransferts(p_nomFichier);
    if (fichierTransferts.bad()) {throw logic_error("fichier introuvable");} //Vérifie si le fichier existe
    string lignesDuFichier;

    while (getline(fichierTransferts, lignesDuFichier))
    {
        if (lignesDuFichier[0] != 'f')
        {
            vector<string> transfertsVec = string_to_vector(lignesDuFichier, ',');
            unsigned int from_station_id = stoi(transfertsVec[0]);
            unsigned int to_station_id = stoi(transfertsVec[1]);
            unsigned int min_transfer_time = stoi(transfertsVec[3]);
            // ajuste le temps de transfert s'il est à 0
            if (min_transfer_time == 0)
            {
                min_transfer_time = 1;
            }
            // pour ajouter un transfert, il faut que les deux stations soient présentent dans m_station
            if ((m_stations.find(from_station_id) != m_stations.end()) &&
            m_stations.find(to_station_id) != m_stations.end())
            {
                auto station = make_tuple(from_station_id, to_station_id, min_transfer_time);
                m_transferts.push_back(station);
            }
        }
    }
    // on vient ajouter les station de transfert dans le conteneur m_stationsDeTransfert en fonction de leur existence
    for (auto & transfert : m_transferts)
    {
        if(m_stations.find(get<0>(transfert)) != m_stations.end())
        {
            m_stationsDeTransfert.insert(get<0>(transfert));
        }
    }
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
        if (lignesDuFichier[0] != 's')
        {
            vector<string> servicesVec = string_to_vector(lignesDuFichier, ',');
            Date dateDeService(stoi(servicesVec[1].substr(0,4)),
                    stoi(servicesVec[1].substr(4,2)),
                    stoi(servicesVec[1].substr(6,2)));

            if (servicesVec[2] == "1" && m_date == dateDeService)
            {
                m_services.insert(servicesVec[0]);
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
    ifstream fichierVoyages(p_nomFichier);
    if (fichierVoyages.bad()) {throw logic_error("fichier introuvable");} //Vérifie si le fichier existe
    string lignesDuFichier;

    while (getline(fichierVoyages, lignesDuFichier))
    {
        if (lignesDuFichier[0] != 'r')
        {
            vector<string> servicesVec = string_to_vector(lignesDuFichier, ',');

            string idVoyage = servicesVec[2];
            unsigned int idLigne = stoi(servicesVec[0]);
            string idService = servicesVec[1];
            string destinationVoyage = servicesVec[3];
            destinationVoyage.erase(remove(destinationVoyage.begin(), destinationVoyage.end(), '\"'),destinationVoyage.end());

            // si le service existe, on ajoute le voyage de le conteneur
            if (m_services.find(idService) != m_services.end())
            {
                Voyage newVoyage(idVoyage, idLigne, idService, destinationVoyage);
                m_voyages[newVoyage.getId()] = newVoyage;
            }
            servicesVec.clear();
        }
    }
}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier) {

    ifstream fichierArrets(p_nomFichier);
    if (fichierArrets.bad()) { throw logic_error("fichier introuvable"); } //Vérifie si le fichier existe
    string lignesDuFichier;

    while (getline(fichierArrets, lignesDuFichier)) {
        // ajout des arrêts au station en fonction du temps demandé
        if (lignesDuFichier[0] != 't')
        {
            vector<string> arretsVec = string_to_vector(lignesDuFichier,',');

            Heure heureArrive(stoi(arretsVec[1].substr(0, 2)),
                              stoi(arretsVec[1].substr(3, 2)),
                              stoi(arretsVec[1].substr(6, 2)));
            Heure heureDepart(stoi(arretsVec[1].substr(0, 2)),
                              stoi(arretsVec[1].substr(3, 2)),
                              stoi(arretsVec[1].substr(6, 2)));

            // on commence par vérifier si les heures corresponde avec ceux de l'objet
            if ((heureArrive >= m_now1) && (heureDepart < m_now2)&& (m_voyages.find(arretsVec[0]) != m_voyages.end()))
            {
                // on ajoute ensuite l'arrêt
                unsigned int p_station_id = stoi(arretsVec[3]);
                Arret::Ptr ptrArret = make_shared<Arret>(p_station_id, heureArrive, heureDepart, stoi(arretsVec[4]),
                                                         arretsVec[0]);
                m_voyages[arretsVec[0]].ajouterArret(ptrArret);
            }
        }
    }
    // Fait une copie de la map, pour ensuite itérer et supprimer les voyages sans arrêts
    map<std::string, Voyage> copieMvoyages = m_voyages; //afin de pouvoir supprimer le contenu de la map
    for (auto const & voyage : copieMvoyages)
    {
        if (voyage.second.getNbArrets() == 0)
        {
            m_voyages.erase(voyage.first);
        }
    }
    // Itère à travers les voyages, et pour chaque, ressort la liste des arrêts
    // Ensuite, on itère dans la liste d'arrêt, on vérifie si la station d'un arrêt existe, si oui, on ajout le shrdPTR
    for (const auto & voyage : m_voyages)
    {
        for (const auto &arret : voyage.second.getArrets())
        {
            m_nbArrets++;
            unsigned int station_id = arret->getStationId();
            if (m_stations.find(station_id) != m_stations.end())
            {
                m_stations.at(station_id).addArret(arret);
            }
        }
    }
    // Fait une copie de la map, pour ensuite itérer et supprimer les stations sans arrêts
    std::map<unsigned int, Station> copieStations = m_stations;
    for (const auto & station : copieStations)
    {
        if(station.second.getNbArrets() == 0)
        {
            m_stations.erase(station.first);
        }
    }
    m_tousLesArretsPresents = true;
}



