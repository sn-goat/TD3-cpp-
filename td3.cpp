//
// Created by Sergile Nyouvop on 2024-02-14.
//
#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "class_td3.h"      // Structures de données pour la collection de films en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <span>

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
    return valeur;
}
#define erreurFataleAssert(message) assert(false&&(message)),terminate()

//DECLARATION DES FONCTIONS NE FAISANT PAS PARTIE DE LA CLASSE
void afficherActeur(const Acteur& acteur);
Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
Acteur* lireActeur(istream& fichier, const  ListeFilms& listeFilms);
Acteur* trouverActeurListeFilms(const ListeFilms& listeFilms, const string& nomActeur);


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

ListeActeurs::~ListeActeurs(){
}
ListeActeurs::ListeActeurs()
        : capacite_(0),
          nElements_(0),
          elements_(nullptr){}

ListeActeurs::ListeActeurs(ListeActeurs&& autre)
    : capacite_(autre.capacite_),
          nElements_(autre.nElements_)
          {
    elements_ = make_unique<Acteur*[]>(capacite_);
    for (int i : range(nElements_)) {
        elements_[i] = autre.elements_[i];
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
        elements_ = make_unique<Acteur*[]>(autre.capacite_);
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

Acteur** ListeActeurs::trouverElements() const{
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

ListeFilms ListeFilms::creerListe(string nomFichier){
    ifstream fichier(nomFichier, ios::binary);
    fichier.exceptions(ios::failbit);

    int nElements = int(lireUintTailleVariable(fichier));

    ListeFilms listeFilms = ListeFilms();
    for (int i = 0; i < nElements; i++) {
        ajouterFilmListeFilms(listeFilms, lireFilm(fichier, listeFilms));
    }

    return listeFilms;
}

void ListeFilms::detruireFilm(Film* film, ListeFilms& listeFilms) {
    enleverFilmListeFilms(listeFilms, film);
    for(int i:range(film->acteurs.trouverNElements())){
        enleverFilmListeFilms(film->acteurs.trouverElements()[i]->joueDans, film);
        if (film->acteurs.trouverElements()[i]->joueDans.trouverNElements() == 0) {
            cout << "Destruction de l'acteur " << film->acteurs.trouverElements()[i]->nom << endl;
            delete[] film->acteurs.trouverElements()[i]->joueDans.elements_;
            delete film->acteurs.trouverElements()[i];
        }
    }
//    for (Acteur* acteur : span(film->acteurs.elements, film->acteurs.nElements)) {
//        enleverFilmListeFilms(acteur->joueDans, film);
//        if (acteur->joueDans.nElements_ == 0) {
//            cout << "Destruction de l'acteur " << acteur->nom << endl;
//            delete[] acteur->joueDans.elements_;
//            delete acteur;
//        }
//    }
//    delete[] film->acteurs.elements;
    delete film;

}

void ListeFilms::detruireListeFilms(ListeFilms& listeFilms) {
    for (int index : range(0, listeFilms.nElements_)) {
        index = 0;
        detruireFilm(listeFilms.elements_[index], listeFilms);
    }
    delete[] listeFilms.elements_;
}


void ListeFilms::afficherListeFilms(const ListeFilms& listeFilms) const {
    static const string ligneDeSeparation = "----------------------------------------\n";
    cout << ligneDeSeparation;

    for (Film* film : span(listeFilms.elements_, listeFilms.nElements_)) {
        cout << "  " << film->titre << ", Realise par " << film->realisateur << ", sorti en " << film->anneeSortie << ", recettes : " << film->recette << ", Acteurs: " << endl;
        for (int i : range(film->acteurs.trouverNElements())) {
            afficherActeur(*film->acteurs.trouverElements()[i]);
        }

        cout << ligneDeSeparation;
    }
}

void ListeFilms::afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur) const{
    const Acteur* acteur = trouverActeurListeFilms(listeFilms, nomActeur);
    if (acteur == nullptr) {
        cout << "Aucun acteur de ce nom" << endl;
    }
    else {
        afficherListeFilms(acteur->joueDans);
    }
}

void ListeActeurs::creerListeActeurs() {
    elements_ = make_unique<Acteur*[]>(capacite_);
    /*for (int i: range(capacite_)) {
        elements_[i] = nullptr;
    }*/
}
void ListeActeurs::definirCapcite(int capacite) {
    capacite_ = capacite;
}
void ListeActeurs::ajouterActeurListeActeur(Acteur* acteur) {
        elements_[nElements_++] = acteur;

}

//////////////////////////////////////////////  FIN DE LIMPLEMENTATION DES METHODES DES CLASSES ///////////////////////////////////////


//////////////////////////////////////////////  FONCTIONS DES STRUCTURES ////////////////////////////////////////////////////////////////////
void afficherActeur(const Acteur& acteur){
    cout << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

Film* lireFilm(istream& fichier, ListeFilms& listeFilms){
    Film film = {};

    film.titre = lireString(fichier);
    film.realisateur = lireString(fichier);
    film.anneeSortie = int(lireUintTailleVariable(fichier));
    film.recette = int(lireUintTailleVariable(fichier));
    int capacite = int(lireUintTailleVariable(fichier));
    film.acteurs = ListeActeurs(capacite);

//    film.acteurs.elements = new Acteur * [film.acteurs.nElements];
    Film* newFilm = new Film();
    newFilm = &film;

    for (int i: range(film.acteurs.trouverCapacite())) {
        Acteur* ptrActeur = lireActeur(fichier, listeFilms);
        film.acteurs.ajouterActeurListeActeur(ptrActeur);
//        film.acteurs.elements[i] = ptrActeur;
        listeFilms.ajouterFilmListeFilms(ptrActeur->joueDans, newFilm);

    }
    return newFilm;
}

Acteur* trouverActeurListeFilms(const ListeFilms& listeFilms, const string& nomActeur) {
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

Acteur* lireActeur(istream& fichier, const  ListeFilms& listeFilms){
    Acteur acteur = {};
    acteur.nom = lireString(fichier);
    acteur.anneeNaissance = int(lireUintTailleVariable(fichier));
    acteur.sexe = char(lireUintTailleVariable(fichier));

    Acteur* acteurExistant = trouverActeurListeFilms(listeFilms, acteur.nom);

    bool acteurEstPtrNull = acteurExistant == nullptr;
    if (acteurEstPtrNull) {
        cout << "Nom de l'acteur ajouté: " << acteur.nom << " " << acteur.anneeNaissance << '\n';
        Acteur* acteurCree = new Acteur;
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
    cout << "  " << film->titre << ", Realise par " << film->realisateur << ", sorti en " << film->anneeSortie << ", recettes : " << film->recette << endl;

    cout << ligneDeSeparation << "Les films sont:" << endl;
    //TODO: Afficher la liste des films.  Il devrait y en avoir 7.
    listeFilms.afficherListeFilms(listeFilms);

    //TODO: Modifier l'année de naissance de Benedict Cumberbatch pour être 1976 (elle était 0 dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des films et des acteurs dans les listes, il faut y aller par son nom.
    Acteur* benedictCumberbatch = trouverActeurListeFilms( listeFilms, "Benedict Cumberbatch" );
    benedictCumberbatch->anneeNaissance = 1976;


    cout << benedictCumberbatch->nom << " " << benedictCumberbatch->anneeNaissance << endl;
    cout << ligneDeSeparation << "Liste des films où Benedict Cumberbatch joue sont:" << endl;

    //TODO: Afficher la liste des films où Benedict Cumberbatch joue.  Il devrait y avoir Le Hobbit et Le jeu de l'imitation.

    listeFilms.afficherFilmographieActeur(listeFilms, "Benedict Cumberbatch");

    //TODO: Détruire et enlever le premier film de la liste (Alien).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les acteurs Tom Skerritt et John Hurt, mais pas Sigourney Weaver puisqu'elle joue aussi dans Avatar.
    listeFilms.detruireFilm(listeFilms.trouverElements()[0], listeFilms);
    cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
    //TODO: Afficher la liste des films.
    listeFilms.afficherListeFilms(listeFilms);

    //TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme
    // (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new"
    // et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes
    // qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

    //TODO: Détruire tout avant de terminer le programme.
    // La bibliothèque de verification_allocation devrait afficher "Aucune fuite detectee."
    // a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque
    // des delete.
    listeFilms.detruireListeFilms(listeFilms);
}
