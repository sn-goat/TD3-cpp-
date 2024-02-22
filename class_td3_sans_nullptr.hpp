#pragma once
// Structures mémoires pour une collection de films.

#include <string>

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
    ListeFilms();
    ~ListeFilms();

    int trouverNElements() const;
    Film** trouverElements() const;


    void afficherFilmographieActeur(const ListeFilms& listeFilms, const std::string& nomActeur) const;
    void ajouterFilmListeFilms(ListeFilms& listeFilms, Film* film);
    void enleverFilmListeFilms(ListeFilms& listeFilms, Film* film);

    friend std::ostream& operator<< (std::ostream& o, const ListeFilms& listeFilms);
    void detruireListeFilms(ListeFilms& listeFilms);
    void detruireFilm(Film* film, ListeFilms& listeFilms);
    ListeFilms creerListe(std::string nomFichier);

private:
    int capacite_;
    int nElements_;
    Film** elements_ = {};

};

struct ListeActeurs {
    int capacite, nElements;
    Acteur** elements; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
};

struct Film
{
    std::string titre, realisateur; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
    int anneeSortie, recette; // Année de sortie et recette globale du film en millions de dollars
    ListeActeurs acteurs;
};

struct Acteur
{
    std::string nom; int anneeNaissance; char sexe;
    ListeFilms joueDans;
};
