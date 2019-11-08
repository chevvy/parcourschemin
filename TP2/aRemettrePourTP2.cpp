//
// Created by Mario Marchand on 16-12-30.
//

#include "ReseauGTFS.h"
#include <sys/time.h>

using namespace std;

//! \brief ajout des arcs dus aux voyages
//! \brief insère les arrêts (associés aux sommets) dans m_arretDuSommet et m_sommetDeArret
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsVoyages(const DonneesGTFS & p_gtfs)
{
	//écrire votre code ici
    int numeroArret = 0;
	for(auto const & voyage : p_gtfs.getVoyages()) // itère dans la liste des voyages présent dans l'objet GTFS
    {
	    auto arretsDuVoyage = voyage.second.getArrets();
	    // TODO est-ce qu'il y a un moyen d'accéder au ptr arret via le set ? oui p-e via arretItr.get(); qui retourne ptr

	    // on vient créer un vecteur pour crisser les ptr dedans pour rendre la manip plus facile

	    vector<Arret> vecteurArrets;
	    for( const auto & arretIter : arretsDuVoyage)
	    {
	        vecteurArrets.push_back(*arretIter);
	    }

        // on stock le premier arrêt dans un premier iterateur, et l'arret précédent dans un deuxième
        // on pourrait aussi le faire avec un stack/queue?
        for( auto arretIter=vecteurArrets.begin(), prevArret = vecteurArrets.end();
	        arretIter != vecteurArrets.end(); prevArret= arretIter, ++arretIter )
        {
	        Arret::Ptr ptrArret = make_shared<Arret>(arretIter->getStationId(), arretIter->getHeureArrivee(), arretIter->getHeureDepart(), arretIter->getNumeroSequence(), arretIter->getVoyageId());
	        // ensuite, on vérifie s'il y a déjè un arc dans le graphe. si pas d'item, on skip (ça veut dire que c'est le premier arrêt
	        if(arretIter == vecteurArrets.begin() || prevArret == vecteurArrets.end()) //si on est au début de la liste de vecteur
	        {
//	           // TODO Purifie moi ça svp
//	            int poids = 0 ; // étant donné que c'est le premier arret, le poids est de 0
//	            m_leGraphe.ajouterArc(numeroArret, numeroArret, poids);
//
//                if (m_sommetDeArret.find(ptrArret) != m_sommetDeArret.end()) // on vérifie que l'arret n,est pas déjà ajouter
//                {
//                    m_arretDuSommet.push_back(ptrArret);// on ajoute au vecteur m_arretDuSommet[size_t arret1] = sharedPrt arret1
//                    m_sommetDeArret[ptrArret] = numeroArret; // on ajoute à la map m_sommetDeArret -> clé = arretPTR et valeur = size_t arret1
//                }

	        }
                // on ajoute au vecteur m_arretDuSommet[size_t arret1] = sharedPrt arret1
                // on ajoute à la map m_sommetDeArret -> clé = arretPTR et valeur = size_t arret1
	        else
            {
                // sinon, on fait la différence de temps entre arret2-arret1 = poids
	            int poids = arretIter->getHeureArrivee() - prevArret->getHeureArrivee();
                // on créer/ajoute l'arc(size_t arret1, size_t arret2, poids)
	            m_leGraphe.ajouterArc(numeroArret,numeroArret+1 , poids);

	            if (m_sommetDeArret.find(ptrArret) != m_sommetDeArret.end()) // on vérifie que l'arret n,est pas déjà ajouter
                {
                    m_arretDuSommet.push_back(ptrArret);
                    m_sommetDeArret[ptrArret] = numeroArret;
                }
	            numeroArret++;
	        }
        }



    }


}


//! \brief ajouts des arcs dus aux transferts entre stations
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsTransferts(const DonneesGTFS & p_gtfs)
{
    std::multimap<int, Arret::Ptr> arretsPourSelection; // contient les arrêts qui seront utilisés pour le tx
    // On commence par itérer sur la liste de transferts
	for(auto const & transfert : p_gtfs.getTransferts())
    {
	    //  <from_station_id, to_station_id, min_transfer_time>
	    int fromStationId = get<0>(transfert);
	    int toStationId = get<1>(transfert);
	    int minTransferTime = get<2>(transfert);
        // ensuite, on trouve les arrêts du "fromStationID"
        for (const auto & arretFrom : p_gtfs.getStations().at(fromStationId).getArrets())
        {
            // ensuite on trouve les arrêts du "toStationID"
            for (const auto & arretTo : p_gtfs.getStations().at(toStationId).getArrets())
            {
                string const & voyageFromID = arretFrom.second->getVoyageId();
                string const & voyageToID = arretTo.second->getVoyageId();
                string ligneFrom = p_gtfs.getVoyages().at(voyageFromID).getId();
                string ligneTo = p_gtfs.getVoyages().at(voyageToID).getId();
                // on vérifie que le transfert ne se fait pas sur la même ligne
                if (ligneFrom != ligneTo)
                {
                    // l’heure d’arrivée de l’arrêt B moins l’heure d’arrivée de l’arrêt A est supérieure ou égale à min_transfer_time. I
                    int differenceTemps =  arretTo.second->getHeureArrivee() - arretFrom.second->getHeureArrivee();
                    if(differenceTemps >= minTransferTime)
                    {
                        // on ajoute à la map de différence de temps
                        arretsPourSelection.insert(make_pair(differenceTemps, arretTo.second));
                    }
                }
            }
            int i = m_sommetDeArret[arretFrom.second];
            int j = m_sommetDeArret[arretsPourSelection.lower_bound(minTransferTime)->second];
            m_leGraphe.ajouterArc(i,j, arretsPourSelection.lower_bound(minTransferTime)->first);

        }
    }
}

//! \brief ajouts des arcs d'une station à elle-même pour les stations qui ne sont pas dans DonneesGTFS::m_stationsDeTransfert
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttente(const DonneesGTFS & p_gtfs)
{
	//écrire votre code ici
}


//! \brief ajoute des arcs au réseau GTFS à partir des données GTFS
//! \brief Il s'agit des arcs allant du point origine vers une station si celle-ci est accessible à pieds et des arcs allant d'une station vers le point destination
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \param[in] p_pointOrigine: les coordonnées GPS du point origine
//! \param[in] p_pointDestination: les coordonnées GPS du point destination
//! \throws logic_error si une incohérence est détecté lors de la construction du graphe
//! \post constuit un réseau GTFS représenté par un graphe orienté pondéré avec poids non négatifs
//! \post assigne la variable m_origine_dest_ajoute à true (car les points orignine et destination font parti du graphe)
//! \post insère dans m_sommetsVersDestination les numéros de sommets connctés au point destination
void ReseauGTFS::ajouterArcsOrigineDestination(const DonneesGTFS &p_gtfs, const Coordonnees &p_pointOrigine,
                                               const Coordonnees &p_pointDestination)
{
	//écrire votre code ici
}

//! \brief Remet ReseauGTFS dans l'était qu'il était avant l'exécution de ReseauGTFS::ajouterArcsOrigineDestination()
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \throws logic_error si une incohérence est détecté lors de la modification du graphe
//! \post Enlève de ReaseauGTFS tous les arcs allant du point source vers un arrêt de station et ceux allant d'un arrêt de station vers la destination
//! \post assigne la variable m_origine_dest_ajoute à false (les points orignine et destination sont enlevés du graphe)
//! \post enlève les données de m_sommetsVersDestination
void ReseauGTFS::enleverArcsOrigineDestination()
{
	//écrire votre code ici
}


