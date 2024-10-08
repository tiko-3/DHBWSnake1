#include <Gosu/Gosu.hpp>
#include <Gosu/AutoLink.hpp>
#include <cstdlib>  // Für rand() und srand()
#include <ctime>    // Für time()
#include <iostream>
#include <cmath>
#include <array>
#include <vector>
#include <tuple>

using namespace std;

class Steuerung;

class Schlange {
private:
    vector<tuple<int, int, int>> segmente;  // Die Schlange als Liste von Segmenten (Stelle der Schlange, x, y Kästchen)
    int richtungX = 1; //-1 nach oben, 1 nach unten
    int richtungY = 0;  // 1 nach rechts, -1 nach links
    Gosu::Color farbe;
    Steuerung* steuerung;

public:
    Schlange(Steuerung* steuerung);
    Gosu::Color gibFarbe() const { return farbe; }
    void bewegen();
    void setzeRichtung(int x, int y);
    bool isstApfel(int apfelX, int apfelY);
    tuple<int, int, int> gibKopf() const { return segmente.front(); }
    const vector<tuple<int, int, int>>& gibSegmente() const { return segmente; }
    int gibRichtungX() { return richtungX; }
    int gibRichtungY() { return richtungY; }

};

class Kaestchen {
private:
    int posX;
    int posY;
    Gosu::Color farbe;

public:
    Kaestchen(int x = 0, int y = 0, Gosu::Color farbe = Gosu::Color::WHITE)
        : posX(x), posY(y), farbe(farbe) {}

    int gibPosX() const { return posX; }
    int gibPosY() const { return posY; }
    Gosu::Color gibFarbe() const { return farbe; }
    void setzePosition(int x, int y) { posX = x; posY = y; }
    void setzeFarbe(Gosu::Color neueFarbe) { farbe = neueFarbe; }
};

class Apfel {
private:
    Steuerung* steuerung;
    int menge = 0;
    int posX = 0;
    int posY = 0;

public:
    Apfel(Steuerung* steuerung);
    std::array<int, 2> randomApfelPos();
    int gibMenge() const { return menge; }
    int gibPosX() const { return posX; }
    int gibPosY() const { return posY; }
    void apfelMengedec() { menge = menge - 1; }
};

class Steuerung {
private:
    const int rasterBreite = 10;
    const int rasterHoehe = 10;
    const int kaestchenGroesse = 50;
    double aktualisierungsZeit = 0.5;

    Kaestchen kaestchen[10][10];
    Apfel* apfel;
    Schlange* schlange;

public:
    Steuerung();
    ~Steuerung();
    int gibGroesseFeld() const { return rasterBreite; }
    Kaestchen& gibKaestchen(int i, int j);
    int gibApfelPosX();
    int gibApfelPosY();
    void apfelEntfernen();
    void apfelPlatzieren();
    void apfelGegessen(int posX, int posY);
    void kollisionMitWand(int kopfX, int kopfY);
    void kollisionMitSichSelbst(int kopfX, int kopfY);
    void verloren();
    double gibaktualisierungsZeit() {return aktualisierungsZeit; }
    void setzteAktualisierungsZeit(int aktualisierungsZeit) {this->aktualisierungsZeit = aktualisierungsZeit;}
    Schlange* gibSchlange() { return schlange; }
};

// Implementierung der Schlange
Schlange::Schlange(Steuerung* steuerung) : steuerung(steuerung) {
    // Schlange startet auf Y-Koordinate 4
    segmente.push_back(make_tuple(0, 4, 4));  // Startposition in der Mitte des Rasters
    farbe = Gosu::Color::GREEN;
}

void Schlange::bewegen() {
    int neuerKopfX = get<1>(segmente.front()) + richtungX;
    int neuerKopfY = get<2>(segmente.front()) + richtungY;

    if (isstApfel(steuerung->gibApfelPosX(), steuerung->gibApfelPosY())) {  //ob der Apfel gegessen wurde
        steuerung->apfelGegessen(neuerKopfX,neuerKopfY);
    }else {
        segmente.pop_back();
    }
    steuerung->kollisionMitWand(neuerKopfX,neuerKopfY);
    steuerung->kollisionMitSichSelbst(neuerKopfX, neuerKopfY);
    segmente.insert(segmente.begin(), make_tuple(0, neuerKopfX, neuerKopfY));

    for (int i = 0; i < segmente.size(); i++) {
        get<0>(segmente[i]) = i;
    }
}

void Schlange::setzeRichtung(int x, int y) {        //x,y neue richtung
    // Verhindert das Umdrehen auf sich selbst
    if ((richtungX != -x || richtungX == 0) && (richtungY != -y || richtungY == 0)) {
        richtungX = x;
        richtungY = y;
    }
}

bool Schlange::isstApfel(int apfelX, int apfelY) {
    int kopfX = get<1>(segmente.front());
    int kopfY = get<2>(segmente.front());

    if (kopfX == apfelX && kopfY == apfelY) {
        return true;
    }
    else {
        return false;
    }
}

Apfel::Apfel(Steuerung* steuerung) : steuerung(steuerung) {
    srand(time(nullptr));
}

array<int, 2> Apfel::randomApfelPos() {
    int max = steuerung->gibGroesseFeld()-2;//-2 weil bei -1 sind wir auf dem letzten max feld, weil wir bei 0 anfangen mit zählen
    bool belegt = true;
    int randX = 4;  // Apfel startet auf der gleichen x-Koordinate wie die Schlange
    int randY = 0;
    std::array<int, 2> rueckgabe = { 0, 0 };

    while (belegt) {
        randX = (rand() % max) +1;  // neue zufällige Position
        randY = (rand() % max) +1 ;
        belegt = false;  // initialisiere als nicht belegt

        const vector<tuple<int, int, int>>& segmente = steuerung->gibSchlange()->gibSegmente();

        for (const auto& segment : segmente) {
            if (get<1>(segment) == randX && get<2>(segment) == randY) {
                belegt = true;  // Position ist belegt
                break;  // keine weitere Prüfung notwendig, Position ist besetzt
            }
        }

        // Wenn die Position nicht belegt ist, speichere sie
        if (!belegt) {
            posX = randX;
            posY = randY;
            rueckgabe = { randX, randY };
            menge++;  // möglicherweise eine Menge von Äpfeln, die du zählst?
        }
    }

    return rueckgabe;
}

void Steuerung::apfelEntfernen() {
    kaestchen[apfel->gibPosX()][apfel->gibPosY()].setzeFarbe(Gosu::Color::WHITE);
   
}

void Steuerung::kollisionMitWand(int kopfX, int kopfY) {
    if (kopfX==0) { //raus oben
        verloren();
    }
    if (kopfY==0) {  //raus links
        verloren();
    }
    if (kopfX ==  rasterBreite-1) { //raus unten
        verloren();
    }
    if (kopfY == rasterHoehe-1) {   //raus rechts
        verloren();
    }
}

void Steuerung::kollisionMitSichSelbst(int kopfX, int kopfY) {
        const vector<tuple<int, int, int>>& segmente = schlange->gibSegmente();
        // Der Kopf ist normalerweise das erste Segment
        for (size_t i = 1; i < segmente.size(); ++i) { // Beginne bei 1, um den Kopf auszuschließen
            if (get<1>(segmente[i]) == kopfX && get<2>(segmente[i]) == kopfY) {
                verloren();  // Kollision mit sich selbst
            }
        }
}

Steuerung::Steuerung() {
    for (int i = 0; i < rasterHoehe; ++i) {
        for (int j = 0; j < rasterBreite; ++j) {
            kaestchen[i][j].setzePosition(j * kaestchenGroesse, i * kaestchenGroesse);
            kaestchen[i][j].setzeFarbe(Gosu::Color::WHITE);
            if (i==0|| j == 0|| i == rasterHoehe - 1|| j == rasterBreite - 1) {  
                kaestchen[i][j].setzeFarbe(Gosu::Color::GRAY);

            }

        }
    }
    apfel = new Apfel(this);
    schlange = new Schlange(this);

    apfelPlatzieren();
}

Steuerung::~Steuerung() {
    delete apfel;
    delete schlange;
}

void Steuerung::verloren() {
    setzteAktualisierungsZeit(10000); //10 minuten 
    cout << "verloren";
}

Kaestchen& Steuerung::gibKaestchen(int i, int j) {
    return kaestchen[i][j];
}
int Steuerung::gibApfelPosX() {
    return apfel->gibPosX();
}
int Steuerung::gibApfelPosY() {
    return apfel->gibPosY();
}
void Steuerung::apfelPlatzieren() { //von Apfel die neue position des Apfels in kästchen gespeichert
    array<int, 2> position = apfel->randomApfelPos();
    kaestchen[position[0]][position[1]].setzeFarbe(Gosu::Color::RED);
}

void Steuerung::apfelGegessen(int posX, int posY) {
    if (kaestchen[apfel->gibPosX()][apfel->gibPosY()].gibFarbe() == Gosu::Color::RED) {
        apfel->apfelMengedec();//Apfel menge wird um eins verringert (wenn es mehrere Äpfel gibt)
        apfelEntfernen();
        apfelPlatzieren();
    }
}

class Oberflaeche : public Gosu::Window {
private:
    Steuerung* steuerung;
    double last_move_time;

public:
    Oberflaeche() : Gosu::Window(800, 600), last_move_time(0) {
        set_caption("Snake");
        steuerung = new Steuerung();
    }

    ~Oberflaeche() {
        delete steuerung;
    }

    void update() override {
        double current_time = Gosu::milliseconds() / 1000.0;
        if (current_time - last_move_time >= steuerung->gibaktualisierungsZeit()) {  // Bewegung alle 0,5 Sekunden
            steuerung->gibSchlange()->bewegen();
            last_move_time = current_time;
        }
    }

    void draw() override {
        int max = steuerung->gibGroesseFeld();
        for (int i = 0; i < max; ++i) {
            for (int j = 0; j < max; ++j) {
                Kaestchen& k = steuerung->gibKaestchen(i, j);
                int x = k.gibPosX();
                int y = k.gibPosY();

                Gosu::Color farbe = k.gibFarbe();

                const vector<tuple<int, int, int>>& segmente = steuerung->gibSchlange()->gibSegmente();
                for (const auto& segment : segmente) {
                    if (get<1>(segment) == i && get<2>(segment) == j) {
                        farbe = Gosu::Color::GREEN;
                        break;
                    }
                }

                Gosu::Graphics::draw_quad(
                    x, y, farbe,
                    x + 50, y, farbe,
                    x + 50, y + 50, farbe,
                    x, y + 50, farbe,
                    0
                );

                Gosu::Graphics::draw_line(x, y, Gosu::Color::BLACK, x + 50, y, Gosu::Color::BLACK, 0);
                Gosu::Graphics::draw_line(x, y + 50, Gosu::Color::BLACK, x + 50, y + 50, Gosu::Color::BLACK, 0);
                Gosu::Graphics::draw_line(x, y, Gosu::Color::BLACK, x, y + 50, Gosu::Color::BLACK, 0);
                Gosu::Graphics::draw_line(x + 50, y, Gosu::Color::BLACK, x + 50, y + 50, Gosu::Color::BLACK, 0);
            }
        }
    }

    void button_down(Gosu::Button btn) override {
        switch (btn) {
        case Gosu::KB_W:  // nach oben
            steuerung->gibSchlange()->setzeRichtung(-1, 0);
            break;
        case Gosu::KB_S:  // nach unten
            steuerung->gibSchlange()->setzeRichtung(1, 0);
            break;
        case Gosu::KB_A:  // nach links
            steuerung->gibSchlange()->setzeRichtung(0, -1);
            break;
        case Gosu::KB_D:  // nach rechts
            steuerung->gibSchlange()->setzeRichtung(0, 1);
            break;
        case Gosu::KB_P:
            cout<<"Pause";
            break;
        default:
            break;
        }
    }

};

int main() {
    Oberflaeche fenster;
    fenster.show();
}

