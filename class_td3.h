//
// Created by Sergile Nyouvop on 2024-02-14.
//

#ifndef TD2_H24_FICHIERS_CLASS_TD3_H
#define TD2_H24_FICHIERS_CLASS_TD3_H

#endif //TD2_H24_FICHIERS_CLASS_TD3_H

#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
    ListeFilms();
    ~ListeFilms();

    int trouverNElements() const;
    Film** trouverElements() const;
    Film** trouverElements();


    void afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur) const;
    void ajouterFilmListeFilms(ListeFilms& listeFilms, Film* film);
    void enleverFilmListeFilms(ListeFilms& listeFilms, Film* film);
    void afficherListeFilms(const ListeFilms& listeFilms) const;
    void detruireListeFilms(ListeFilms& listeFilms);
    void detruireFilm(Film* film, ListeFilms& listeFilms);
    ListeFilms creerListe(std::string nomFichier);

private:
    int capacite_;
    int nElements_;
    Film** elements_ = {};

};

class ListeActeurs {
public:
    ListeActeurs();
    ListeActeurs(int capacite);

    ListeActeurs(ListeActeurs&& listeActeurs);
    ListeActeurs& operator=(ListeActeurs&& listeActeurs);

    ~ListeActeurs();

    int trouverNElements() const;
    int trouverCapacite() const;
    Acteur** trouverElements() const;

    void definirCapcite(int capacite);
    void creerListeActeurs();
    void ajouterActeurListeActeur(Acteur* acteur);


private:
    int capacite_;
    int nElements_;
    unique_ptr<Acteur*[]> elements_;




};
//struct ListeActeurs {
//    int capacite, nElements;
//    Acteur** elements; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
//};

struct Film
{
    string titre, realisateur; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
    int anneeSortie, recette; // Année de sortie et recette globale du film en millions de dollars
    ListeActeurs acteurs;
};

struct Acteur
{
    string nom; int anneeNaissance; char sexe;
    ListeFilms joueDans;
};
