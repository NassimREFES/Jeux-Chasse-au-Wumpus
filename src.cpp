// exo 12 ch18 - 18/02/2013
// chasse au wumpus sur console

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <Windows.h>
#include <sstream>
#include <locale>
#include <stdexcept>

#define NOMBRE_DE_SALLE 60
#define NOMBRE_TUNNEL_PAR_SALLE 3
#define NOMBRE_MAXIMUM_DE_FLECHE 5
#define NOMBRE_MAXIMUM_DE_PUITS 3
#define NOMBRE_MAXIMUM_DE_CHAUVES_SOURIS 2

using namespace std;

const char joueur = 'J';
const char wumpus = 'W';
const char puits			= 'P';
const string chauves_souris	= "CS";

void Color(int couleurDuTexte, int couleurDeFond) 
// fonction d'affichage de couleurs
{
	HANDLE H = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(H, couleurDeFond * 16 + couleurDuTexte);
}

inline void keep_window_open()
{
	cout << "\nEnter any key to quit\n";
	char ch;
	cin >> ch;
	return;
}

inline void error(const string& msg)
{
	throw runtime_error(msg);
}

inline int randint(int max) { return rand() % max; }

inline int randint(int min, int max) { return randint(max - min) + min; }

class Salle
{
public :
	enum Salle_type { 
		joueur=0, puits, chauves_souris, wumpus, rien
	};

	Salle();
	Salle(const Salle&);
	Salle& operator=(const Salle&);
	explicit Salle(short numero_de_salle, Salle_type type_de_salle=rien);
	~Salle() { delete[] tunnel; }

	void change_salle(short i) { numero = i; }
	short numero_salle() const { return numero; }

	const short* les_tunnel() const { return tunnel; }
	short* les_tunnel() { return tunnel; }

	void change_type_salle(Salle_type s) { st = s; }
	Salle_type type_de_salle() const { return st; }
private :
	Salle_type st;
	short numero;
	short* tunnel;

	void copie(const Salle&);
};

Salle::Salle()
	:st(rien), numero(1)
{
	tunnel = new short[NOMBRE_TUNNEL_PAR_SALLE];
	for (int i = 0; i < NOMBRE_TUNNEL_PAR_SALLE; ++i)
		tunnel[i] = 0;
}

Salle::Salle(const Salle& s)
{
	st = s.st;
	numero = s.numero;
	tunnel = new short[NOMBRE_TUNNEL_PAR_SALLE];

	copie(s);
}

Salle::Salle(short numero_de_salle, Salle_type s)
	: st(s), numero(numero_de_salle)
{
	tunnel = new short[NOMBRE_TUNNEL_PAR_SALLE];
	for (int i = 0; i < NOMBRE_TUNNEL_PAR_SALLE; ++i)
		tunnel[i] = 0;
}

Salle& Salle::operator=(const Salle& s)
{
	if (this == &s) return *this;

	st = s.st;
	numero = s.numero;
	
	copie(s);

	return *this;
}

void Salle::copie(const Salle& s)
{
	for (int i = 0; i < NOMBRE_TUNNEL_PAR_SALLE; ++i)
		tunnel[i] = s.tunnel[i];
}

class Personnage
{
public :
	enum Nature_personnage { mort, vivant };

	void change_sante(Nature_personnage np) { ma_sante = np; }
	Nature_personnage sante() const { return ma_sante; }

	void deplace(const Salle& s) { ma_salle = s; }
	const Salle& salle() const { return ma_salle; }

	void change_type_salle(Salle::Salle_type s) { ma_salle.change_type_salle(s); }
	Salle::Salle_type type_de_salle() const { return ma_salle.type_de_salle(); }

	void change_nom_personnage(const string& s) { nom_personnage = s; }
	string nom_du_personnage() const { return nom_personnage; }

protected :
	Personnage() : ma_sante(vivant), ma_salle(0), nom_personnage("") { }

private :
	Nature_personnage ma_sante;
	Salle ma_salle;
	string nom_personnage;

	Personnage(const Personnage&);
	Personnage& operator=(const Personnage&);
};

struct Joueur : Personnage
{
	explicit Joueur(const Salle& salle);
	Joueur(const Salle& salle, const string& nom);

	void deplace(const Salle& s) { Personnage::deplace(s); change_type_salle(Salle::joueur); }

	short nombre_de_fleche_restante() const { return fleche; }
	void tirer_une_fleche();
	void reinitialiser_nombre_fleche() { fleche = NOMBRE_MAXIMUM_DE_FLECHE; }
private :
	short fleche;
};

Joueur::Joueur(const Salle& s)
: fleche(NOMBRE_MAXIMUM_DE_FLECHE)
{
	change_nom_personnage(string(1, joueur));
	deplace(s);
}

Joueur::Joueur(const Salle& s, const string& nom)
: fleche(NOMBRE_MAXIMUM_DE_FLECHE)
{
	change_nom_personnage(nom);
	deplace(s);
}

void Joueur::tirer_une_fleche()
{
	if (fleche==0)
		cout << "Ya plus de fleche.";
	else
		fleche -= 1;
}

struct Ennemie : Personnage
{
	explicit Ennemie(const Salle& salle);
	Ennemie(const Salle& salle, const string& nom);

	void deplace(const Salle& s) { Personnage::deplace(s); change_type_salle(Salle::wumpus); }
};

Ennemie::Ennemie(const Salle& s)
{
	change_nom_personnage(string(1, wumpus));
	deplace(s);
}

Ennemie::Ennemie(const Salle& s, const string& nom)
{
	change_nom_personnage(nom);
	deplace(s);
}

struct Piege {
	enum Piege_type {
		puits, chauves_souris
	};

	Piege() : pt(puits) { }
	Piege(const Salle& s, Piege_type p);

	void deplace(const Salle& s);

	Salle ma_salle() const { return salle; }
 
	void change_type_de_piege(Piege_type p);
	Piege_type type_de_piege() const { return pt; }
private :
	Piege_type pt;
	Salle salle;
};

Piege::Piege(const Salle& s, Piege_type p) 
	: pt(p), salle(s)
{
	if (p == puits) salle.change_type_salle(Salle::puits);
	else salle.change_type_salle(Salle::chauves_souris);
}

void Piege::deplace(const Salle& s)
{
	salle = s;
	change_type_de_piege(pt);
}

void Piege::change_type_de_piege(Piege_type p)
{
	pt = p;
	if (pt == puits) salle.change_type_salle(Salle::puits);
	else salle.change_type_salle(Salle::chauves_souris);
}

class Terrain
{
	friend ostream& operator<<(ostream&, const Terrain&);
public :
	Terrain();
	~Terrain() { delete[] salles; }

	short taille_de_terrain() const { return taille_terrain; }

	void change_salle(int i, Salle s) { salles[i] = s; }
	Salle salle(int i) const { return salles[i]; }

	void change_type_salle(int i, Salle::Salle_type st) { salles[i].change_type_salle(st); }
private :
	short taille_terrain;
	Salle* salles;

	void cree_terrain();
	bool genere_tunnels(short*, short salle_courante);
};

Terrain::Terrain()
: taille_terrain(NOMBRE_DE_SALLE)
{
	salles = new Salle[NOMBRE_DE_SALLE];
	cree_terrain();
}

void Terrain::cree_terrain()
{
	short* p=0;
	for (int i = 0; i < NOMBRE_DE_SALLE; ++i) {
		salles[i].change_salle(i);

		// généré les tunnels
		p = salles[i].les_tunnel();
		if (!genere_tunnels(p, i)) 
			error("Les tunnels de la caverne n ont pas pu etre genere.");
	}
}

bool Terrain::genere_tunnels(short* p, short salle_courante)
{
	if (p == 0) return false;

	for (int i = 0; i < NOMBRE_TUNNEL_PAR_SALLE; ++i) {
		p[i] = randint(1, NOMBRE_DE_SALLE);

		while (p[i] == salle_courante)
			p[i] = randint(1, NOMBRE_DE_SALLE);

		if (i == 1)
		while (p[i] == p[i - 1] || p[i] == salle_courante)
			p[i] = randint(1, NOMBRE_DE_SALLE);

		else if (i == 2)
		while (p[i] == p[i - 1] || p[i] == p[i - 2] || p[i] == salle_courante)
			p[i] = randint(1, NOMBRE_DE_SALLE);
	}

	return true;
}

ostream& operator<<(ostream& os, const Terrain& t)
{
	for (int i = 1; i <= NOMBRE_DE_SALLE; ++i) {
		os << setw(7);
		Color(12, 0);
		if (t.salle(i-1).type_de_salle() == Salle::joueur)
			os << joueur;
		else if (t.salle(i - 1).type_de_salle() == Salle::wumpus)
			os << wumpus;
		else if (t.salle(i - 1).type_de_salle() == Salle::puits)
			os << puits;
		else if (t.salle(i - 1).type_de_salle() == Salle::chauves_souris)
			os << chauves_souris;
		else {
			Color(15, 0);
			os << t.salle(i - 1).numero_salle();
		}

		if (i%10==0) os << "\n\n\n";
	}
	Color(15, 0);

	return os;
}

class Chasse_au_wumpus
{
public :
	Chasse_au_wumpus();
	~Chasse_au_wumpus();

	void affiche_caverne_avec_tout_membres() const { cout << caverne << '\n'; }
	void affiche_caverne() const;
	void demarer_jeux();
	void aide() const;
private :
	Terrain caverne;
	Joueur joueur;
	Ennemie ennemie;
	Piege* piege_puits;
	Piege* piege_chauves_souris;

	bool genere_emplacement_piege();
	bool genere_emplacement_puits();
	bool genere_emplacement_chauves_souris();

	void afficher_danger(Salle::Salle_type s);
	void danger_salle_adjacente();

	void bouger(const string& reponse, const short * const tunnels);
	bool bouger_valid(const string& reponse);

	void tirer(const string& reponse);
	bool tirer_valid(const string& reponse, vector<short>& sn);

	void diriger_le_wumpus_vers_joueur();
};

Chasse_au_wumpus::Chasse_au_wumpus()
	: caverne(Terrain()),
	joueur(caverne.salle(randint(0, NOMBRE_DE_SALLE - 1))), 
	ennemie(caverne.salle(randint(0, NOMBRE_DE_SALLE - 1))),
	piege_puits(new Piege[NOMBRE_MAXIMUM_DE_PUITS]),
	piege_chauves_souris(new Piege[NOMBRE_MAXIMUM_DE_CHAUVES_SOURIS])
{
	while (joueur.salle().numero_salle() == ennemie.salle().numero_salle())
		ennemie.deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));

	genere_emplacement_piege();

	caverne.change_salle(joueur.salle().numero_salle(), joueur.salle());
	caverne.change_salle(ennemie.salle().numero_salle(), ennemie.salle());

	for (int i = 0; i < NOMBRE_MAXIMUM_DE_PUITS; ++i)
		caverne.change_salle(piege_puits[i].ma_salle().numero_salle(),
								piege_puits[i].ma_salle());

	for (int i = 0; i < NOMBRE_MAXIMUM_DE_CHAUVES_SOURIS; ++i)
		caverne.change_salle(piege_chauves_souris[i].ma_salle().numero_salle(),
			piege_chauves_souris[i].ma_salle());
}

Chasse_au_wumpus::~Chasse_au_wumpus()
{
	delete[] piege_puits;
	delete[] piege_chauves_souris;
}

void Chasse_au_wumpus::affiche_caverne() const
{
	for (int i = 1; i <= NOMBRE_DE_SALLE; ++i) {
		cout << setw(7);
		Color(12, 0);
		if (caverne.salle(i - 1).type_de_salle() == Salle::joueur)
			cout << ::joueur;
		else {
			Color(15, 0);
			cout << caverne.salle(i - 1).numero_salle();
		}

		if (i % 10 == 0) cout << "\n\n\n";
		Color(15, 0);
	}
}

bool Chasse_au_wumpus::genere_emplacement_piege()
{
	return genere_emplacement_puits() && genere_emplacement_chauves_souris();
}	

bool Chasse_au_wumpus::genere_emplacement_puits()
{
	int a = joueur.salle().numero_salle();
	int b = ennemie.salle().numero_salle();

	for (int i = 0; i < NOMBRE_MAXIMUM_DE_PUITS; ++i) {
		piege_puits[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
		int x = piege_puits[i].ma_salle().numero_salle();
		while (x == a || x == b) {
			piege_puits[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
			x = piege_puits[i].ma_salle().numero_salle();
		}

		if (i == 1)
			while (x == a || x == b || x == piege_puits[i-1].ma_salle().numero_salle()) 
			{
				piege_puits[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
				x = piege_puits[i].ma_salle().numero_salle();
			}
		else if (i == 2)
			while (x == a || x == b || x == piege_puits[i - 1].ma_salle().numero_salle() 
				|| x == piege_puits[i - 2].ma_salle().numero_salle())
			{
				piege_puits[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
				x = piege_puits[i].ma_salle().numero_salle();
			}
		piege_puits[i].change_type_de_piege(Piege::puits);
	}
	return true;
}

bool Chasse_au_wumpus::genere_emplacement_chauves_souris()
{
	int a = joueur.salle().numero_salle();
	int b = ennemie.salle().numero_salle();
	int c1 = piege_puits[0].ma_salle().numero_salle();
	int c2 = piege_puits[1].ma_salle().numero_salle();
	int c3 = piege_puits[2].ma_salle().numero_salle();

	for (int i = 0; i < NOMBRE_MAXIMUM_DE_CHAUVES_SOURIS; ++i) {
		piege_chauves_souris[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
		int x = piege_chauves_souris[i].ma_salle().numero_salle();
		while (x == a || x == b || x == c1 || x == c2 || x == c3) {
			piege_chauves_souris[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
			x = piege_chauves_souris[i].ma_salle().numero_salle();
		}
		if (i == 1) 
		while (x == a || x == b || x == c1 || x == c2 || x == c3
			|| x == piege_chauves_souris[i-1].ma_salle().numero_salle()) 
		{
			piege_chauves_souris[i].deplace(caverne.salle(randint(1, NOMBRE_DE_SALLE - 1)));
			x = piege_chauves_souris[i].ma_salle().numero_salle();
		}
		piege_chauves_souris[i].change_type_de_piege(Piege::chauves_souris);
	}
	return true;
}

bool Chasse_au_wumpus::bouger_valid(const string& reponse)
{
	if (reponse[0] != 'm') return false;

	for (int i = 1; i<reponse.length(); ++i) 
		if (!isdigit(reponse[i]))
			return false;

	return true;
}

void Chasse_au_wumpus::bouger(const string& reponse, const short * const tunnels)
{
	if (!bouger_valid(reponse)) { cout << "Invalid format pour bouger.\n"; return; }
	char ch;
	short salle;
	istringstream iss(reponse);
	iss >> ch;
	iss >> salle;

	if (salle < 0 || salle > 59) 
		cout << "Salle invalid.\n";
	else if (!(salle == tunnels[0] || salle == tunnels[1] || salle == tunnels[2]))
		cout << "Il existe pas de tunnel vers la salle: " << salle << '\n';
	else {
		if (caverne.salle(salle).type_de_salle() == Salle::wumpus ||
			caverne.salle(salle).type_de_salle() == Salle::puits) {
			joueur.change_sante(Personnage::mort);
			return;
		}
		else if (caverne.salle(salle).type_de_salle() == Salle::chauves_souris) {
			for (int i = 0; i < NOMBRE_MAXIMUM_DE_CHAUVES_SOURIS; ++i)
			if (piege_chauves_souris[i].ma_salle().numero_salle() == salle) {
				caverne.change_type_salle(piege_chauves_souris[i].ma_salle().numero_salle(), Salle::rien);
				piege_chauves_souris[i].deplace(caverne.salle(randint(0, NOMBRE_DE_SALLE - 1)));
				caverne.change_salle(piege_chauves_souris[i].ma_salle().numero_salle(), 
										piege_chauves_souris[i].ma_salle());
				break;
			}
			salle = randint(0, NOMBRE_DE_SALLE - 1);
		}
		caverne.change_type_salle(joueur.salle().numero_salle(), Salle::rien);
		joueur.deplace(caverne.salle(salle));
		caverne.change_salle(joueur.salle().numero_salle(), joueur.salle());
	}
}

bool Chasse_au_wumpus::tirer_valid(const string& reponse, vector<short>& sn)
{
	for (int i = 0; i < reponse.length(); ++i)
	if (reponse[i] == ' ')
		return false;

	istringstream iss(reponse);
	char ch;
	short nombre;
	short nombre_de_tirer_union = 0;

	iss >> ch;
	if (ch != 's') return false;
	while (iss) {
		if (!(iss >> nombre)) return false;
		sn.push_back(nombre);
		if (sn.size() > 3)
		{
			cout << "La porté d'une fleche est de trois salles.\n";
			break;
		}
		if (iss >> ch && ch != '-') return false;
		++nombre_de_tirer_union;
		if (nombre_de_tirer_union > 3) return false;
	}

	return true;
}

void Chasse_au_wumpus::tirer(const string& reponse)
{
	vector<short> sn;
	if (!tirer_valid(reponse, sn)) { cout << "Invalid format pour tirer.\n"; return; }

	if (joueur.nombre_de_fleche_restante() == 0) { cout << "Vous n'avez plus de fleche.\n"; return; }
	else joueur.tirer_une_fleche();

	for (int i = 0; i < sn.size(); ++i) {
		if (sn[i] < 0 || 59 < sn[i]) {
			cout << "La salle " << sn[i] << " n'existe pas.\n";
		}
		else {
			if (caverne.salle(sn[i]).type_de_salle() == Salle::wumpus)
				ennemie.change_sante(Personnage::mort);
		}
	}

	if (ennemie.sante() == Personnage::vivant) diriger_le_wumpus_vers_joueur();
}

void Chasse_au_wumpus::diriger_le_wumpus_vers_joueur()
{
	short nsj = joueur.salle().numero_salle();
	short haut = nsj - 10;
	short bas = nsj + 10;
	short droite = nsj + 1;
	short gauche = nsj - 1;

	if (9 < nsj && nsj < 60 && caverne.salle(haut).type_de_salle() == Salle::rien) {
		caverne.change_type_salle(ennemie.salle().numero_salle(), Salle::rien);
		ennemie.deplace(caverne.salle(haut));
	}
	else if (0 <= nsj && nsj < 50
		&& caverne.salle(bas).type_de_salle() == Salle::rien) {
		caverne.change_type_salle(ennemie.salle().numero_salle(), Salle::rien);
		ennemie.deplace(caverne.salle(bas));
	}
	else if (0 <= nsj && nsj < 60 && nsj % 10 != 9
		&& caverne.salle(droite).type_de_salle() == Salle::rien) {
		caverne.change_type_salle(ennemie.salle().numero_salle(), Salle::rien);
		ennemie.deplace(caverne.salle(droite));
	}
	else if (0 < nsj && nsj < 60 && nsj % 10 != 0
		&& caverne.salle(gauche).type_de_salle() == Salle::rien) {
		caverne.change_type_salle(ennemie.salle().numero_salle(), Salle::rien);
		ennemie.deplace(caverne.salle(gauche));
	}

	caverne.change_salle(ennemie.salle().numero_salle(), ennemie.salle());
}

void Chasse_au_wumpus::demarer_jeux()
{
	system("CLS");
	bool continuer = true;
	int choix_numero_salle = 0;
	const short* tunnels = 0;
	string reponse;
	while (continuer) {
		affiche_caverne();
		tunnels = joueur.salle().les_tunnel();
		Color(14, 0); danger_salle_adjacente(); Color(15, 0);
		cout << "Vouv vous trouvez en salle " << joueur.salle().numero_salle()
			<< "\nil ya des tunnels vers les salles " << tunnels[0] << ", " << tunnels[1] << " et " << tunnels[2]
			<< "\nbouger ou tirer? ";

		getline(cin, reponse);

		switch (reponse[0]) {
		case 'm': bouger(reponse, tunnels); break;
		case 's': tirer(reponse); break;
		default : cout << "Invalid commande de jeu.\n";
		}

		if (joueur.sante() == Personnage::mort) {
			Color(12, 0); cout << "Vous avez perdu\n"; Color(15, 0);
			continuer = false;
		}

		else if (ennemie.sante() == Personnage::mort) {
			Color(9, 0); cout << "Vous avez tuer le wumpus. Ta Gagnez\n"; Color(15, 0);
			continuer = false;
		}
		system("PAUSE");
	}
}

void Chasse_au_wumpus::danger_salle_adjacente()
{
	short nsj = joueur.salle().numero_salle();
	short haut = nsj - 10;
	short bas  = nsj + 10;
	short droite = nsj + 1;
	short gauche = nsj - 1;
	short haut_droite = haut + 1;
	short haut_gauche = haut - 1;
	short bas_droite = bas + 1;
	short bas_gauche = bas - 1;

	if (9<nsj && nsj<60) afficher_danger(caverne.salle(haut).type_de_salle());
	if (0<=nsj && nsj<50) afficher_danger(caverne.salle(bas).type_de_salle());
	if (0<=nsj && nsj<60 && nsj % 10 != 9) afficher_danger(caverne.salle(droite).type_de_salle());
	if (0<nsj && nsj<60 && nsj % 10 != 0) afficher_danger(caverne.salle(gauche).type_de_salle());
	if (9<nsj && nsj<60 && nsj % 10 != 9) afficher_danger(caverne.salle(haut_droite).type_de_salle());
	if (9<nsj && nsj<60 && nsj % 10 != 0) afficher_danger(caverne.salle(haut_gauche).type_de_salle());
	if (0<=nsj && nsj<50 && nsj % 10 != 9) afficher_danger(caverne.salle(bas_droite).type_de_salle());
	if (0<=nsj && nsj<50 && nsj % 10 != 0) afficher_danger(caverne.salle(bas_gauche).type_de_salle());
}

void Chasse_au_wumpus::afficher_danger(Salle::Salle_type s)
{
	switch (s) {
	case Salle::chauves_souris : cout << "<<J'entends une chauve-souris>>\n"; break;
	case Salle::puits: cout << "<<Je sens un courant d'aire>>\n"; break;
	case Salle::wumpus: cout << "<<Je sens le Wumpus>>\n"; break;
	}
}

void Chasse_au_wumpus::aide() const
{
	system("CLS");
	Color(13, 0); cout << "\t----------[ Chasse au wumpus ]----------\n\n"; Color(15, 0);
	cout << "Le joueur commence a jouer dans une caverne \nqui contien 3 tunnels vers d'autre salle.\n"
		<< "Pour bouger de salle en salle on utlise la commande m13 (move en salle 13)\n"
		<< "si la salle courante a un tunnels vers la salle choisi.\n"
		<< "Pour tirer vers les salle on utilise la commande\ns1-2-3 (shoot en salle 1,2 et 3)\n"
		<< "un tire de fleche attire le wumpus vers une salle adjacente a la votre.\n"
		<< "la porté d'une fleche est de trois salles\n\n"
		<< "Type de peige:\n"
		<< "puits : si vous tomber dedans vous mourrez\n"
		<< "chauve-souris : vous emporte dans une autre salle\n\n"
		<< "ennemie : le wumpus\n"
		<< "wumpus : si tu entre dans la salle ou se trouve ou il rentre dans la votre vous mourrez\n"
		<< "joueur : vous ! ;)\n"
		<< "vous disposez de 5 fleche au depart.\n";
	system("PAUSE");
	system("CLS");
}

void menu()
{
	short choix;
	bool quitter = false;
	while (!quitter) {
		Chasse_au_wumpus p;
		system("CLS");
		Color(13, 0); cout << "\t----------[ Chasse au wumpus ]----------\n\n"; Color(15, 0);
		cout << "Menu: \n\n";
		cout << "1 - Commencez le jeu\n"
			<< "2 - Aide\n"
			<< "3 - Quitter\n\n"
			<< "Votre choix?\n";
		do {
			cout << "> ";
			cin >> choix;
		} while (choix<1 || choix>3);

		cin.ignore(120, '\n');

		switch (choix) {
		case 1:
		{
				  p.demarer_jeux();
				  system("CLS");
				  Color(14, 0);  cout << "la map avec tout les membres\n\n"; Color(15, 0);
				  p.affiche_caverne_avec_tout_membres();
				  system("PAUSE");
				  break;
		}
		case 2: p.aide(); break;
		case 3: quitter = true;  break;
		}
	}
}

int main()
try{
	setlocale(LC_ALL, "");
	srand(time(NULL));
	Color(15, 0);

	menu();

	return 0;
}
catch (exception& e)
{
	cerr << e.what() << '\n';
	keep_window_open();
	return 1;
}


