#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de films en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <span>
#include <sstream>
#include "include/cppitertools/range.hpp"


#include "include/bibliotheque_cours.hpp"
#include "include/verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "include/debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
    T valeur{};
    fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
    assert(!fichier.fail());
    return valeur;
}

#define erreurFataleAssert(message) assert(false&&(message)),terminate()

//DECLARATION DES FONCTIONS NE FAISANT PAS PARTIE DE LA CLASSE
void afficherActeur(ostream& o, const Acteur& acteur);
Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
shared_ptr<Acteur> lireActeur(istream& fichier, const  ListeFilms& listeFilms);
shared_ptr<Acteur> trouverActeurListeFilms(const ListeFilms& listeFilms, const string& nomActeur);


static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
    uint8_t entete = lireType<uint8_t>(fichier);
    switch (entete) {
    case enteteTailleVariableDeBase + 0: return lireType<uint8_t>(fichier);
    case enteteTailleVariableDeBase + 1: return lireType<uint16_t>(fichier);
    case enteteTailleVariableDeBase + 2: return lireType<uint32_t>(fichier);
    default:
        erreurFataleAssert("Tentative de lire un entier de taille variable alors que le fichier contient autre chose à cet emplacement.");
    }
}

string lireString(istream& fichier)
{
    string texte;
    texte.resize(lireUintTailleVariable(fichier));
    fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
    return texte;
}

#pragma endregion//}


ListeFilms::ListeFilms()
    : capacite_(0),
    nElements_(0),
    elements_(nullptr) {}


ListeFilms::~ListeFilms() {
}
int ListeFilms::trouverNElements() const {
    return nElements_;
}

Film** ListeFilms::trouverElements()  const {
    return elements_;
}
Film** ListeFilms::trouverElements() {
    return elements_;
}

ListeActeurs::~ListeActeurs() {
}
ListeActeurs::ListeActeurs()
    : capacite_(0),
    nElements_(0),
    elements_(nullptr) {}

ListeActeurs::ListeActeurs(const ListeActeurs& listeActeurs)
    : capacite_(listeActeurs.capacite_),
    nElements_(listeActeurs.nElements_)
{
    elements_ = make_unique<shared_ptr<Acteur>[]>(capacite_);
    for (int i : range(nElements_)) {
        elements_[i] = listeActeurs.elements_[i];
    }

}
ListeActeurs::ListeActeurs(int capacite) {
    capacite_ = capacite;
    nElements_ = 0;
    creerListeActeurs();
}

ListeActeurs& ListeActeurs::operator=(ListeActeurs&& autre) {
    if (this != &autre) {
        capacite_ = autre.capacite_;
        nElements_ = autre.nElements_;
        elements_ = make_unique<shared_ptr<Acteur>[]>(autre.capacite_);
        for (int i : range(autre.nElements_)) {
            elements_[i] = autre.elements_[i];
        }
    }
    return *this;
}

int ListeActeurs::trouverNElements() const {
    return nElements_;
}
int ListeActeurs::trouverCapacite() const {
    return capacite_;
}

shared_ptr<Acteur>* ListeActeurs::trouverElements() const {
    return elements_.get();
}





//////////////////////////////////////////////  DEBUT DE LIMPLEMENTATION DES METHODES DES CLASSES /////////////////////////////////////////


void ListeFilms::ajouterFilmListeFilms(ListeFilms& listeFilms, Film* film) {
    if (listeFilms.capacite_ <= (listeFilms.nElements_ + 1)) {
        int nouvelleCapacite = (listeFilms.capacite_ == 0) ? 1 : 2 * listeFilms.capacite_;
        Film** tableauDouble = new Film * [nouvelleCapacite];

        if (listeFilms.nElements_ != 0) {
            for (int i = 0; i < listeFilms.nElements_; i++) {
                tableauDouble[i] = listeFilms.elements_[i];
            }
        }
        delete[] listeFilms.elements_;
        listeFilms.elements_ = tableauDouble;
        listeFilms.capacite_ = nouvelleCapacite;
    }
    listeFilms.elements_[listeFilms.nElements_] = film;
    cout << "Nom du film ajouté: " << film->titre << endl;
    listeFilms.nElements_++;
}

void ListeFilms::enleverFilmListeFilms(ListeFilms& listeFilms, Film* film) {
    for (int i = 0; i < listeFilms.nElements_; ++i) {
        if (listeFilms.elements_[i] == film) {
            for (int j = i; j < listeFilms.nElements_ - 1; ++j) {
                listeFilms.elements_[j] = listeFilms.elements_[j + 1];
            }
        }
    }
    listeFilms.nElements_--;

}

ListeFilms ListeFilms::creerListe(const string& nomFichier) {
    ifstream fichier(nomFichier, ios::binary);
    if (!fichier.is_open()) {
        cerr << "Erreur : Impossible d'ouvrir le fichier " << nomFichier << endl;
        return ListeFilms();
    }
    try {
        fichier.exceptions(ios::failbit);
        // Code susceptible de lever une exception
        int nElements = int(lireUintTailleVariable(fichier));

        ListeFilms listeFilms = ListeFilms();
        for (int i = 0; i < nElements; i++) {
            ajouterFilmListeFilms(listeFilms, lireFilm(fichier, listeFilms));
        }
        return listeFilms;
    }
    catch (const std::ios_base::failure& e) {
        cerr << "Erreur d'entrée-sortie : " << e.what() << endl;
        // Autres actions en cas d'erreur, si nécessaire.
    }
    catch (const std::exception& e) {
        cerr << "Autre exception : " << e.what() << endl;
        // Autres actions en cas d'erreur, si nécessaire.
    }

    // En cas d'erreur, renvoyer une liste vide ou une liste d'éléments par défaut, selon votre logique.
    return ListeFilms();
}

void ListeFilms::detruireFilm(Film* film, ListeFilms& listeFilms) {
    enleverFilmListeFilms(listeFilms, film);
    cout << "Destruction Film " << film->titre << endl;
    // Supprimer le film une fois que les acteurs associés ont été traités.
    delete film;
}

void ListeFilms::detruireListeFilms(ListeFilms& listeFilms) {
    for (int index : range(0, listeFilms.nElements_)) {
        index = 0;
        detruireFilm(listeFilms.elements_[index], listeFilms);
        
    }
    delete[] listeFilms.elements_;
}

template<typename PredicatUniaire>
auto ListeFilms::trouverFilm(const PredicatUniaire& critere){
    for(auto film : span(elements_, nElements_)){
        if(critere(film)){
            return film;
        }
    }
}

void afficherActeur(ostream& o, const Acteur& acteur) {
    o << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

ostream& operator<< (ostream& o, const ListeFilms& listeFilms) {
    static const string ligneDeSeparation = "----------------------------------------\n";
    o << ligneDeSeparation;
    for (Film* film : span(listeFilms.elements_, listeFilms.nElements_)) {
        o << "  " << film->titre << ", Realise par " << film->realisateur << ", sorti en " << film->anneeSortie << ", recettes : " << film->recette <<"M$"<< ", Acteurs: " << endl;
        for (int i : range(film->acteurs.trouverNElements())) {
                afficherActeur(o ,*film->acteurs.trouverElements()[i]);
        }

        o << ligneDeSeparation;

    }
    return o;

}
/*
void ListeFilms::afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur) const {
    const shared_ptr<Acteur>acteur = trouverActeurListeFilms(listeFilms, nomActeur);
    if (acteur == nullptr) {
        cout << "Aucun acteur de ce nom" << endl;
    }
    else {
        afficherListeFilms(acteur->joueDans);
    }
}
*/
void ListeActeurs::creerListeActeurs() {
    elements_ = make_unique<shared_ptr<Acteur>[]>(capacite_);
    /*for (int i: range(capacite_)) {
        elements_[i] = nullptr;
    }*/
}
//void ListeActeurs::definirCapcite(int capacite) {
//    capacite_ = capacite;
//}
void ListeActeurs::ajouterActeurListeActeur(shared_ptr<Acteur> acteur) {
    elements_[nElements_++] = acteur;

}

//////////////////////////////////////////////  FIN DE LIMPLEMENTATION DES METHODES DES CLASSES ///////////////////////////////////////


//////////////////////////////////////////////  FONCTIONS DES STRUCTURES ////////////////////////////////////////////////////////////////////
//void afficherActeur(ostream& o, const Acteur& acteur) {
//    o << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
//}

ostream& operator<< (ostream& o, const Film* film) {
    static const string ligneDeSeparation = "----------------------------------------\n";
    o << "  " << film->titre << ", Realise par " << film->realisateur << ", sorti en " << film->anneeSortie << ", recettes : " << film->recette << "M$" << " Acteurs:  " << endl;
    for (int i : range(film->acteurs.trouverNElements())) {
    
        afficherActeur(o, *film->acteurs.trouverElements()[i]);
    }
    o << ligneDeSeparation ;
    return o;
}
ostream& operator<< (ostream& o, const Film& film) {
    static const string ligneDeSeparation = "----------------------------------------\n";
    o << "  " << film.titre << ", Realise par " << film.realisateur << ", sorti en " << film.anneeSortie << ", recettes : " << film.recette << "M$" << " Acteurs:  " << endl;
    for (int i : range(film.acteurs.trouverNElements())) {

        afficherActeur(o, *film.acteurs.trouverElements()[i]);
    }
    o << ligneDeSeparation;
    return o;
    
}
Film* lireFilm(istream& fichier, ListeFilms& listeFilms) {
    Film* newFilm = new Film();  // Allouez dynamiquement un nouvel objet Film.

    newFilm->titre = lireString(fichier);
    newFilm->realisateur = lireString(fichier);
    newFilm->anneeSortie = int(lireUintTailleVariable(fichier));
    newFilm->recette = int(lireUintTailleVariable(fichier));
    int capacite = int(lireUintTailleVariable(fichier));
    newFilm->acteurs = ListeActeurs(capacite);

    for (int i : range(newFilm->acteurs.trouverCapacite())) {
        shared_ptr<Acteur> ptrActeur = lireActeur(fichier, listeFilms);
        newFilm->acteurs.ajouterActeurListeActeur(ptrActeur);
        //listeFilms.ajouterFilmListeFilms(ptrActeur->joueDans, newFilm);
    }

    return newFilm;  // Retournez le pointeur du nouvel objet Film.
}

shared_ptr<Acteur> trouverActeurListeFilms(const ListeFilms& listeFilms, const string& nomActeur) {
    for (Film* filmDansListe : span(listeFilms.trouverElements(), listeFilms.trouverNElements())) {
        for (int valeur : range(filmDansListe->acteurs.trouverNElements())) {
            bool acteurTrouve = filmDansListe->acteurs.trouverElements()[valeur]->nom == nomActeur;
            if (acteurTrouve) {
                return filmDansListe->acteurs.trouverElements()[valeur];
            }
        }
    }
    return nullptr;
}

shared_ptr<Acteur> lireActeur(istream& fichier, const  ListeFilms& listeFilms) {
    Acteur acteur = {};
    acteur.nom = lireString(fichier);
    acteur.anneeNaissance = int(lireUintTailleVariable(fichier));
    acteur.sexe = char(lireUintTailleVariable(fichier));

    shared_ptr<Acteur>acteurExistant = trouverActeurListeFilms(listeFilms, acteur.nom);

    bool acteurEstPtrNull = acteurExistant == nullptr;
    if (acteurEstPtrNull) {
        cout << "Nom de l'acteur ajouté: " << acteur.nom << " " << acteur.anneeNaissance << '\n';
        shared_ptr<Acteur>acteurCree = make_unique<Acteur>(acteur);
        *acteurCree = acteur;
        return acteurCree;
    }
    else {
        return acteurExistant;
    }
}

/////////////////////////////////////////////////////////  MAIN ////////////////////////////////////////////////////////////////////////////////

int main()
{
    bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

    static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";


    //TODO: La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.
    // Devrait afficher les noms de 20 acteurs sans doublons
    // (par l'affichage pour fins de débogage dans votre fonction lireActeur).
    ListeFilms listeFilms = listeFilms.creerListe("films.bin");

    cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
    //TODO: Afficher le premier film de la liste.  Devrait être Alien.
    Film* film = listeFilms.trouverElements()[0];
    cout << film;
    
    cout << ligneDeSeparation << "Les films sont:" << endl;
    //TODO: Afficher la liste des films.  Il devrait y en avoir 7.
    cout << listeFilms;

    //TODO: Modifier l'année de naissance de Benedict Cumberbatch pour être 1976 (elle était 0 dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des films et des acteurs dans les listes, il faut y aller par son nom.
    shared_ptr<Acteur>benedictCumberbatch = trouverActeurListeFilms(listeFilms, "Benedict Cumberbatch");
    benedictCumberbatch->anneeNaissance = 1976;


    cout << benedictCumberbatch->nom << " " << benedictCumberbatch->anneeNaissance << endl;
    cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;

    //TODO: Afficher la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.
    
    //listeFilms.afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");
    //skylien
    // 1. Film skylien = listeFilms[0]; ou Film skylien = *listeFilms[0]; selon ce qui fait du sens.
    Film skylien = *listeFilms.trouverElements()[0]; // ou Film* skylien = *listeFilms.trouverElements();

    // 2. Changer le titre du film skylien pour "Skylien".
    skylien.titre = "Skylien";

    // 3. Changer le premier acteur du film skylien pour le premier acteur de listeFilms[1].
    Film secondFilm = *listeFilms.trouverElements()[1];
    skylien.acteurs.trouverElements()[0] = secondFilm.acteurs.trouverElements()[0];

    // 4. Changer le nom du premier acteur de skylien pour "Daniel Wroughton Craig".
    skylien.acteurs.trouverElements()[0]->nom = "Daniel Wroughton Craig";

    // 5. Afficher skylien, listeFilms[0] et listeFilms[1].
    cout << "Affichage de skylien:" << endl;
    cout << skylien;

    cout << "Affichage de listeFilms[0]:" << endl;
    cout << listeFilms.trouverElements()[0];

    cout << "Affichage de listeFilms[1]:" << endl;
    cout << listeFilms.trouverElements()[1];

    //TODO: Détruire et enlever le premier film de la liste (Alien).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les acteurs Tom Skerritt et John Hurt, mais pas Sigourney Weaver puisqu'elle joue aussi dans Avatar.
    listeFilms.detruireFilm(listeFilms.trouverElements()[0], listeFilms);
    cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
    //TODO: Afficher la liste des films.
    cout << listeFilms;

    

    //TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme
    // (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new"
    // et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes
    // qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

    //TODO: Changer la fonction afficherFilm pour qu’on puisse afficher comme ceci:
    // cout << unFilm << unAutreFilm; Et que ça fonctionne si on veut l’afficher sur
    // autre chose que cout, par exemple dans un ostringstream (dans <sstream> ) ou dans un fichier avec ofstream
    cout << ligneDeSeparation << "Test de l'operateur '<<':" << endl;
    ostringstream tamponStringStream;

    tamponStringStream << listeFilms.trouverElements()[5];
    string filmEnString = tamponStringStream.str();

    cout << filmEnString << listeFilms.trouverElements()[3];

    ofstream fichier("films.txt");

    fichier << filmEnString << listeFilms;

    cout << ligneDeSeparation << "Trouver un film avec une fonction lambda pour indiquer le critère:" << endl;

    int recette = 955;
    cout << listeFilms.trouverFilm([x= recette](auto v){ return v->recette == x;});

    //TODO: Détruire tout avant de terminer le programme.
    // La bibliothèque de verification_allocation devrait afficher "Aucune fuite detectee."
    // a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque
    // des delete.
    cout << ligneDeSeparation << "Destruction de listeFilms: " << endl;
    listeFilms.detruireListeFilms(listeFilms);
}
