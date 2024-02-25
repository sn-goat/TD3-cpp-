
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

template <typename T>
class Liste {
public:
    Liste();
    Liste(int capacite);


    Liste(const Liste& liste);
    Liste& operator=(const Liste& liste);
    shared_ptr<T> operator[](int index);
    const shared_ptr<T>& operator[](int index) const;



    ~Liste();

    int trouverNElements() const;
    int trouverCapacite() const;
    void reallocation(int nouvelleCapacite);

//    shared_ptr<T>* trouverElements() const;


    //void definirCapcite(int capacite);
    
//    void creerListe();
    void ajouterElement(const shared_ptr<T>& element);


private:
    int capacite_ = 0;
    int nElements_ = 0;
    unique_ptr<shared_ptr<T>[]> elements_;




};

struct Acteur
{
    string nom;
    int anneeNaissance = 0;
    char sexe;
    //ListeFilms joueDans;
};
//struct Liste {
//    int capacite, nElements;
//    Acteur** elements; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
//};

struct Film
{
    string titre, realisateur; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
    int anneeSortie, recette; // Année de sortie et recette globale du film en millions de dollars
    Liste<Acteur> acteurs;
};
