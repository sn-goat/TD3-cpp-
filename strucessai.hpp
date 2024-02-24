
#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
using namespace std;

struct Film; // Permet d'utiliser les types alors qu'ils seront défini après.

struct Acteur;


class ListeFilms {
public:
    ListeFilms();
    ~ListeFilms();

    int trouverNElements() const;
    Film** trouverElements() const;
    Film** trouverElements();

    friend ostream& operator<< (ostream& o, const Film& film);
    
    //void afficherFilmographieActeur(const ListeFilms& listeFilms, const string& nomActeur) const;
    friend ostream& operator<< (ostream& o, const ListeFilms& listeFilms);
    void ajouterFilmListeFilms(ListeFilms& listeFilms, Film* film);
    void enleverFilmListeFilms(ListeFilms& listeFilms, Film* film);

    template<typename PredicatUniaire>
    auto trouverFilm(const PredicatUniaire& critere);
    //void afficherListeFilms(const ListeFilms& listeFilms) const;
    void detruireListeFilms(ListeFilms& listeFilms);
    void detruireFilm(Film* film, ListeFilms& listeFilms);
    ListeFilms creerListe(const string& nomFichier);

private:
    int capacite_;
    int nElements_;
    Film** elements_ = {};

};

class ListeActeurs {
public:
    ListeActeurs();
    ListeActeurs(int capacite);

    ListeActeurs(const ListeActeurs& listeActeurs);
    ListeActeurs& operator=(ListeActeurs&& listeActeurs);

    ~ListeActeurs();

    int trouverNElements() const;
    int trouverCapacite() const;
    shared_ptr<Acteur>* trouverElements() const;

    //void definirCapcite(int capacite);
    void creerListeActeurs();
    void ajouterActeurListeActeur(shared_ptr<Acteur> acteur);


private:
    int capacite_ = 0;
    int nElements_ = 0;
    unique_ptr<shared_ptr<Acteur>[]> elements_;




};
struct Acteur
{
    string nom;
    int anneeNaissance = 0;
    char sexe;
    //ListeFilms joueDans;
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
