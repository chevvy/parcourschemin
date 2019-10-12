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
            // Ligne(unsigned int p_id, const std::string & p_numero, const std::string & p_description, const CategorieBus& p_categorie);

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
        if (lignesDuFichier[0] != 's') // pour éviter la première ligne du fichier (à changer ? TODO)
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
            // 0-route_id, 1-service_id, 2-trip_id, 3-trip_headsign, 4-trip_short_name, 5-direction_id, 6- block_id, 7-shape_id, 8-wheelchair_accessible
            // Voyage(const std::string & p_id[2], unsigned int p_ligne_id[0], const std::string & p_service_id[1], const std::string & p_destination[3]);
            // enlever les "" de destination
            string idVoyage = servicesVec[2];
            unsigned int idLigne = stoi(servicesVec[0]);
            string idService = servicesVec[1];
            string destinationVoyage = servicesVec[3];
            destinationVoyage.erase(remove(destinationVoyage.begin(), destinationVoyage.end(), '\"'), destinationVoyage.end());

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
        if (lignesDuFichier[0] != 't') // pour éviter la première ligne du fichier (à changer ? TODO)
        {
            vector<string> arretsVec = string_to_vector(lignesDuFichier,
                                                        ','); // est-ce necessaire de convertir en vec? TODO
            // 0 - trip_id, 1 - arrival_time, 2 - departure_time, 3 - stop_id, 4 - stop_sequence, 5 - pickup_type, 6 - drop_off_type
            // faire objet heure arrival time
            // faire objet heure departure time
            // if arrival time => now 1 && departure time <= now 2
            // ajout de l'arrêt
            Heure heureArrive(stoi(arretsVec[1].substr(0, 2)),
                              stoi(arretsVec[1].substr(3, 2)),
                              stoi(arretsVec[1].substr(6, 2)));
            Heure heureDepart(stoi(arretsVec[1].substr(0, 2)),
                              stoi(arretsVec[1].substr(3, 2)),
                              stoi(arretsVec[1].substr(6, 2)));

            if ((heureArrive >= m_now1) && (heureDepart <= m_now2)
                && (m_voyages.find(arretsVec[0]) != m_voyages.end())) {
                // 0 - trip_id, 1 - arrival_time, 2 - departure_time, 3 - stop_id, 4 - stop_sequence, 5 - pickup_type, 6 - drop_off_type
                // 	Arret(unsigned int p_station_id, const Heure & p_heure_arrivee, const Heure & p_heure_depart,
                //          unsigned int p_numero_sequence, const std::string & p_voyage_id);
                // cout << stoi(arretsVec[3]) << heureArrive << heureDepart << arretsVec[4] << arretsVec[0];
                // Arret newArret(stoi(arretsVec[3]), heureArrive, heureDepart, stoi(arretsVec[4]), arretsVec[0]);
                unsigned int p_station_id = stoi(arretsVec[3]);
                Arret::Ptr ptrArret = make_shared<Arret>(p_station_id, heureArrive, heureDepart, stoi(arretsVec[4]),
                                                         arretsVec[0]);
                m_voyages[arretsVec[0]].ajouterArret(ptrArret);
                // m_voyages.at(arretsVec[0]).ajouterArret(ptrArret);

            }
        }
    }
    // nettoyage de m_voyages

    map<std::string, Voyage> copieMvoyages = m_voyages; //afin de pouvoir supprimer le contenu de la map
    for (auto const & voyage : copieMvoyages)
    {
        if (voyage.second.getNbArrets() == 0)
        {
            m_voyages.erase(voyage.first);
        }
    }

    //        if (voyageM.second.getNbArrets() > 0)
    //        {
    //           for (auto & arretsM : voyageM.second.getArrets())
    //           {
    //               unsigned int station_id = arretsM->getStationId();
    //               m_stations.at(station_id).addArret(arretsM);
    //           }
     //       }


    // Ajouter copie ptr aux arrêt de la station m_station TODO
    //     for ( auto & stationM : m_stations) // est-ce je renomme stationM ? TODO
    //    {
    //        if(stationM.first == p_station_id)
    //        {
    //            stationM.second.addArret(ptrArret);
    //        }
    //    }


}



