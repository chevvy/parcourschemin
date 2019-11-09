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

	    vector<shared_ptr<Arret>> vecteurArrets;
	    for( const auto & arretIter : arretsDuVoyage)
	    {
	        vecteurArrets.push_back(arretIter);
	    }

        // on stock le premier arrêt dans un premier iterateur, et l'arret précédent dans un deuxième
        // on pourrait aussi le faire avec un stack/queue?
        for( auto arretIter=vecteurArrets.begin(), prevArret = vecteurArrets.end();
	        arretIter != vecteurArrets.end(); prevArret= arretIter, ++arretIter )
        {
	        // ensuite, on vérifie s'il y a déjè un arc dans le graphe. si pas d'item, on skip (ça veut dire que c'est le premier arrêt
	        if(arretIter == vecteurArrets.begin() || prevArret == vecteurArrets.end()) //si on est au début de la liste de vecteur
	        {
//	           // TODO Purifie moi ça svp
	        }
                // on ajoute au vecteur m_arretDuSommet[size_t arret1] = sharedPrt arret1
                // on ajoute à la map m_sommetDeArret -> clé = arretPTR et valeur = size_t arret1
	        else
            {
                // sinon, on fait la différence de temps entre arret2-arret1 = poids
	            int poids = arretIter->get()->getHeureArrivee() - prevArret->get()->getHeureArrivee();
                // on créer/ajoute l'arc(size_t arret1, size_t arret2, poids)
                if (m_sommetDeArret.find(arretIter.operator*()) != m_sommetDeArret.end()) // on vérifie que l'arret n,est pas déjà ajouter
                {
                    m_sommetDeArret[arretIter.operator*()] = numeroArret;
                    m_arretDuSommet.push_back(arretIter.operator*());
                }
                m_leGraphe.ajouterArc(numeroArret,numeroArret+1 , poids);
                cout << "ajout arc " << numeroArret << numeroArret+1 << endl;
                numeroArret++;
	        }
        }



    }


}


//! \brief ajouts des arcs dus aux transferts entre stations
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsTransferts(const DonneesGTFS & p_gtfs)
{
    std::multimap<int, shared_ptr<Arret>> arretsPourSelection; // contient les arrêts qui seront utilisés pour le tx
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
                string ligneFrom = p_gtfs.getVoyages().at(voyageFromID).getId(); // TODO à checker si c'est chill pour le ID ?
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
            cout <<"ajout de l'arc " <<i << j << arretFrom.second <<endl;

        }
    }
}

//! \brief ajouts des arcs d'une station à elle-même pour les stations qui ne sont pas dans DonneesGTFS::m_stationsDeTransfert
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttente(const DonneesGTFS & p_gtfs)
{
    for (auto const & station : p_gtfs.getStations())
    {
        if (p_gtfs.getStationsDeTransfert().find(station.second.getId()) == p_gtfs.getStationsDeTransfert().end())
        {
            // on met les arrêts dans un vecteur pour faciliter la prochaine manipulation
            vector<shared_ptr<Arret>> vecteurArrets;
            for( const auto & arretIter : station.second.getArrets())
            {
                vecteurArrets.push_back(arretIter.second);
            }

            for( auto arretIter=vecteurArrets.begin(), prevArret = vecteurArrets.end();
                 arretIter != vecteurArrets.end(); prevArret= arretIter, ++arretIter )
            {
                // TODO à corriger ça (changer pour que ça soit juste un if plutôt que IF else)
                if(arretIter == vecteurArrets.begin() || prevArret == vecteurArrets.end()) //si on est au début de la liste de vecteur
                {
                    // on est au debut de la liste
                }

                else
                {
                    int poids = arretIter->get()->getHeureArrivee() - prevArret->get()->getHeureArrivee(); // prevArret = A arretiter = B
                    //- ajoutera un arc dans m_leGraphe entre l’arrêt A et l’arrêt B de cette station si et seulement si les conditions suivantes sont satisfaites :
                    //• l’heure d’arrivée de l’arrêt B moins l’heure d’arrivée de l’arrêt A est supérieure ou égale à delaisMinArcsAttente (qui est un membre constant de ReseauGTFS fixé à 300 secondes).
                    if ( poids >= delaisMinArcsAttente)
                    {
                        // on va chercher le ID du voyage associé à l'arrêt A (prev)
                        string const & voyageIdA = prevArret->get()->getVoyageId();
                        // on va chercher le int de la ligne associé au voyage
                        unsigned int numDeLigneA = p_gtfs.getVoyages().at(voyageIdA).getLigne();
                        // finalement, on vient chercher le string de la ligne
                        string nomDeligneA = p_gtfs.getLignes().find(numDeLigneA)->second.getNumero();

                        // on va chercher le ID du voyage associé à l'arrêt B (arretIter)
                        string const & voyageIdB = arretIter->get()->getVoyageId();
                        unsigned int numDeLigneB = p_gtfs.getVoyages().at(voyageIdB).getLigne();
                        string nomDeligneB = p_gtfs.getLignes().find(numDeLigneB)->second.getNumero();

                        //• Le numéro de ligne du voyage de l’arrêt A est différent de celui de l’arrêt B.
                        if(nomDeligneA != nomDeligneB)
                        {
                            // on vient vérifier les conditions pour tout vecteur C (CAD -> ¬A ¬B)

                            bool arretsRespecteConditions = true;
                            for( auto arretIterCC=vecteurArrets.begin() ; arretIterCC != vecteurArrets.end(); ++arretIterCC )
                            {
                                // vérifie que l'arrêt c n'est pas l'arret A ou B
                                if (arretIterCC != arretIter || arretIterCC != prevArret)
                                {
                                    unsigned int numDeLigneC = p_gtfs.getVoyages().at(arretIterCC->get()->getVoyageId()).getLigne();
                                    string nomDeligneC = p_gtfs.getLignes().find(numDeLigneC)->second.getNumero();
                                    //• Il n’existe pas un autre arrêt C à cette station dont :
                                    //o le numéro de ligne est le même que celui de B
                                    bool numdeLigneCetBidentique = (numDeLigneB == numDeLigneC);
                                    //o et dont l’heure d’arrivée est plus petit que celui de B
                                    bool heureArriveCplusPetitQueB = arretIterCC->get()->getHeureArrivee() < arretIter->get()->getHeureArrivee();
                                    //o et est plus grand ou égale à celui de A + delaisMinArcsAttente.
                                    Heure heureArriveAplusDelais(prevArret->get()->getHeureArrivee());
                                    heureArriveAplusDelais.add_secondes(delaisMinArcsAttente*60);
                                    bool heureArriveCplusGrandQueAetDelais = arretIterCC->get()->getHeureArrivee() >= (heureArriveAplusDelais);

                                    if (numdeLigneCetBidentique && heureArriveCplusPetitQueB && heureArriveCplusGrandQueAetDelais)
                                    {
                                        arretsRespecteConditions = false;
                                    }
                                }
                                if (arretsRespecteConditions)
                                {
                                    // on créer/ajoute l'arc(size_t arret1, size_t arret2, poids)
                                    int i = m_sommetDeArret[* prevArret];
                                    int j = m_sommetDeArret[* arretIter];
                                    m_leGraphe.ajouterArc(i, j, poids);
                                    // cout << "Arc attente ajouté au i = " << m_sommetDeArret[* prevArret] << " et j = " << m_sommetDeArret[* arretIter] <<endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
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


