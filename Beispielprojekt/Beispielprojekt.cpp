#include <Gosu/Gosu.hpp>
#include <Gosu/AutoLink.hpp>
#include <cstdlib>  // Für rand() und srand()
#include <ctime>    // Für time()
#include <iostream>
#include <cmath>
#include <array>
using namespace std;

class Steuerung;  // Vorwärtsdeklaration der Klasse Steuerung

class Schlange {
private:
    int tempo;
    Gosu::Color farbe;
    vector <int> kopf;      //der Vector merkt sich die Position des Kopfes über die Kästchen
    vector <int> schluss;
public:
    Schlange();
    Gosu::Color gibFarbe() const { //wenn es zwei schlangen gibt
        return farbe; 
    } 
    void setzeTempo(int tempo) {
        this->tempo = tempo;
    }

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

public:
    Steuerung();
    ~Steuerung();
    int gibGroesseFeld() const { return rasterBreite; }
    Kaestchen& gibKaestchen(int i, int j);
    void apfelPlatzieren();
    void apfelGegessen(int posX, int posY);
};

// Implementierungen

Apfel::Apfel(Steuerung* steuerung) : steuerung(steuerung) {
    srand(time(nullptr));
}

std::array<int, 2> Apfel::randomApfelPos() {
    int max = steuerung->gibGroesseFeld();
    bool belegt = true;
    int randX = 0;
    int randY = 0;
    std::array<int, 2> rückgabe = { 0, 0 };
    while (belegt) {
        randX = rand() % max;
        randY = rand() % max;
        Kaestchen& k = steuerung->gibKaestchen(randX, randY);
        if (k.gibFarbe() == Gosu::Color::WHITE) {
            belegt = false;
            posX = randX;
            posY = randY;
            rückgabe = { randX, randY };
            menge++;
        }
    }
    return rückgabe;
}

Steuerung::Steuerung() {
    // Korrekte Positionierung der Kästchen
    for (int i = 0; i < rasterHoehe; ++i) {
        for (int j = 0; j < rasterBreite; ++j) {
            kaestchen[i][j].setzePosition(j * kaestchenGroesse, i * kaestchenGroesse);
            kaestchen[i][j].setzeFarbe(Gosu::Color::WHITE);
        }
    }
    apfel = new Apfel(this);
    apfelPlatzieren();
}

Steuerung::~Steuerung() {
    delete apfel;
}

Kaestchen& Steuerung::gibKaestchen(int i, int j) {
    return kaestchen[i][j];
}

void Steuerung::apfelPlatzieren() {
    array<int, 2> position = apfel->randomApfelPos();
    kaestchen[position[0]][position[1]].setzeFarbe(Gosu::Color::RED);
}

void Steuerung::apfelGegessen(int posX, int posY) {
    if (kaestchen[apfel->gibPosX()][apfel->gibPosY()].gibFarbe() == Gosu::Color::RED) {
        apfel->apfelMengedec();
        kaestchen[posX][posY].setzeFarbe(Gosu::Color::WHITE);
        apfelPlatzieren();
    }
}

class Oberflaeche : public Gosu::Window {
private:
    Steuerung* steuerung;

public:
    Oberflaeche() : Gosu::Window(800, 600) {
        set_caption("Snake");
        steuerung = new Steuerung();
    }

    ~Oberflaeche() {
        delete steuerung;
    }

    void draw() override {
        int max = steuerung->gibGroesseFeld();
        for (int i = 0; i < max; ++i) {
            for (int j = 0; j < max; ++j) {
                Kaestchen& k = steuerung->gibKaestchen(i, j);
                int x = k.gibPosX();
                int y = k.gibPosY();
                Gosu::Graphics::draw_quad(
                    x, y, k.gibFarbe(),
                    x + 50, y, k.gibFarbe(),
                    x + 50, y + 50, k.gibFarbe(),
                    x, y + 50, k.gibFarbe(),
                    0
                );

                // Zeichne die schwarzen Ränder
                Gosu::Graphics::draw_line(x, y, Gosu::Color::BLACK, x + 50, y, Gosu::Color::BLACK, 0);
                Gosu::Graphics::draw_line(x, y + 50, Gosu::Color::BLACK, x + 50, y + 50, Gosu::Color::BLACK, 0);
                Gosu::Graphics::draw_line(x, y, Gosu::Color::BLACK, x, y + 50, Gosu::Color::BLACK, 0);
                Gosu::Graphics::draw_line(x + 50, y, Gosu::Color::BLACK, x + 50, y + 50, Gosu::Color::BLACK, 0);
            }
        }
    }
};

int main() {
    Oberflaeche fenster;
    fenster.show();
}
