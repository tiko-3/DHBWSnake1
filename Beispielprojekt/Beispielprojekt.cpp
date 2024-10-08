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
    int richtungX = 1;  // 1: nach rechts, -1: nach links, 0: keine Bewegung in X-Richtung
    int richtungY = 0;  // 1: nach unten, -1: nach oben, 0: keine Bewegung in Y-Richtung
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
    const int rasterBreite = 8;
    const int rasterHoehe = 8;
    const int kaestchenGroesse = 50;

    Kaestchen kaestchen[8][8];
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

    if (isstApfel(steuerung->gibApfelPosX(), steuerung->gibApfelPosY())) {
        steuerung->apfelGegessen(neuerKopfX,neuerKopfY);
    }
    else {
        segmente.pop_back();
    }

    segmente.insert(segmente.begin(), make_tuple(0, neuerKopfX, neuerKopfY));

    for (int i = 0; i < segmente.size(); i++) {
        get<0>(segmente[i]) = i;
    }
}

void Schlange::setzeRichtung(int x, int y) {
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
    int max = steuerung->gibGroesseFeld();
    bool belegt = true;
    int randX = 4;// Apfel startet auf der gleichen x-Koordinate wie die Schlange
    int randY = 0; 
    std::array<int, 2> rueckgabe = { 0, 0 };
    while (belegt) {
        randX = rand() % max;//neue random position
        randY = rand() % max;

        for (int i = 0; i < max; ++i) {
            for (int j = 0; j < max; ++j) {
                Kaestchen& k = steuerung->gibKaestchen(randX, randY);   //kästchen bei random position
                int x = k.gibPosX();
                int y = k.gibPosY();

                const vector<tuple<int, int, int>>& segmente = steuerung->gibSchlange()->gibSegmente();
                for (const auto& segment : segmente) {
                    if (get<1>(segment) != randX && get<2>(segment) != randY) {
                        belegt = false;
                        posX = randX;
                        posY = randY;
                        rueckgabe = { randX, randY };
                        menge++;
                        break;
                    }
                }
            }
        }
    }
    return rueckgabe;
}

void Steuerung::apfelEntfernen() {
    kaestchen[apfel->gibPosX()][apfel->gibPosY()].setzeFarbe(Gosu::Color::WHITE);
   
}

Steuerung::Steuerung() {
    for (int i = 0; i < rasterHoehe; ++i) {
        for (int j = 0; j < rasterBreite; ++j) {
            kaestchen[i][j].setzePosition(j * kaestchenGroesse, i * kaestchenGroesse);
            kaestchen[i][j].setzeFarbe(Gosu::Color::WHITE);
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


Kaestchen& Steuerung::gibKaestchen(int i, int j) {
    return kaestchen[i][j];
}
int Steuerung::gibApfelPosX() {
    return apfel->gibPosX();
}
int Steuerung::gibApfelPosY() {
    return apfel->gibPosY();
}
void Steuerung::apfelPlatzieren() {
    std::array<int, 2> position = apfel->randomApfelPos();
    kaestchen[position[0]][position[1]].setzeFarbe(Gosu::Color::RED);
}

void Steuerung::apfelGegessen(int posX, int posY) {
    if (kaestchen[apfel->gibPosX()][apfel->gibPosY()].gibFarbe() == Gosu::Color::RED) {
        apfel->apfelMengedec();
        kaestchen[posX][posY].setzeFarbe(Gosu::Color::WHITE);//apfel weiß setzten funktioniert nicht
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
        if (current_time - last_move_time >= 0.5) {  // Bewegung alle 0,5 Sekunden
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
        default:
            break;
        }
    }
};

int main() {
    Oberflaeche fenster;
    fenster.show();
}

