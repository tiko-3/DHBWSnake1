#include <Gosu/Gosu.hpp>
#include <Gosu/AutoLink.hpp>
#include <cstdlib>  // Für rand() und srand()
#include <ctime>    // Für time()
#include <iostream>
#include <cmath>
using namespace std;


class Apfel;  // Vorwärtsdeklaration der Klasse Apfel

class Kaestchen {
private:
    int posX;        // X-Position des Kästchens
    int posY;        // Y-Position des Kästchens   
    Gosu::Color farbe; // Farbe des Kästchens

public:
    // Konstruktor
    Kaestchen(int x = 0, int y = 0, Gosu::Color farbe = Gosu::Color::WHITE)
        : posX(x), posY(y), farbe(farbe) {}

    // Getter für die Position und Farbe
    int gibPosX() const {
        return posX;
    }
    int gibPosY() const {
        return posY;
    }
    Gosu::Color gibFarbe() const {
        return farbe;
    }

    // Setter für die Position und Farbe
    void setzePosition(int x, int y) {
        posX = x;
        posY = y;
    }

    void setzeFarbe(Gosu::Color neueFarbe) {
        farbe = neueFarbe;
    }
};

class Steuerung {
private:
    const int rasterBreite = 8;  // Breite des 2D-Feldes
    const int rasterHoehe = 8;   // Höhe des 2D-Feldes
    const int kaestchenGroesse = 50; // Größe der Kästchen

    Kaestchen kaestchen[8][8]; // 2D-Array von Kästchen-Objekten
    Apfel* apfel;  // Zeiger auf das Apfel-Objekt

public:
    // Konstruktor: Initialisiere alle Kästchen mit einer Position und einer Farbe
    Steuerung() {
        for (int i = 0; i < rasterHoehe; ++i) {
            for (int j = 0; j < rasterBreite; ++j) {
                kaestchen[i][j].setzePosition(j * kaestchenGroesse, i * kaestchenGroesse); // Position setzen
                kaestchen[i][j].setzeFarbe(Gosu::Color::WHITE); // Standardfarbe setzen
            }
        }

        // Dynamische Initialisierung des Apfels
        apfel = new Apfel(this);  // Übergibt die Steuerung als Argument an den Apfel
        apfelPlatzieren();  // Apfel platzieren
    }

    ~Steuerung() {
        delete apfel;  // Speicher für das Apfel-Objekt freigeben
    }

    int gibGroesseFeld() const {  // Feldgröße geben
        return rasterBreite;
    }

    // Methode zum Abrufen eines Kästchens aus dem Array
    Kaestchen& gibKaestchen(int i, int j) {
        return kaestchen[i][j];
    }

    void apfelPlatzieren() {    
        array<int, 2> position = apfel->randomApfelPos();  // Korrektes Aufrufen der Methode
        kaestchen[position[0]][position[1]].setzeFarbe(Gosu::Color::RED);  // Kästchen färben
    }
    void kollision() {

    }
    //apfelgegessen hat fehler bei if, es soll schauen ob an der Kopfposition der APfel ist
    void apfelGegessen(int posX, int posY) {   //bekommt aktuelle Position des Kopfes mitgegeben
        if (kaestchen[apfel->gibPosX()][apfel->gibPosY()].gibFarbe() == Gosu::Color::RED) { //schaut ob an der Kopfposition ein Apfel ist
            apfel->apfelMengedec(); //apfel menge minus -1
            kaestchen[posX][posY].setzeFarbe(Gosu::Color::WHITE);  // Kästchen färben
            apfelPlatzieren();
        }
    }
};

class Apfel {
private:
    int menge = 0;
    Steuerung* steuerung;  // Verweis auf Steuerung
    int posX = 0;
    int posY = 0;

public:
    // Konstruktor, der die Steuerung übergeben bekommt
    Apfel(Steuerung* steuerung) : steuerung(steuerung) {
        // Zufallsgenerator initialisieren
        srand(time(nullptr));
    }

    array<int, 2> randomApfelPos() {
        int max = steuerung->gibGroesseFeld();
        bool belegt = true;
        int randX = 0;
        int randY = 0;
        array<int, 2> rückgabe = { 0, 0 };  // Initialisierung
        while (belegt == true) {    // Neue Position generieren, solange belegt
            randX = rand() % max;
            randY = rand() % max;
            Kaestchen& k = steuerung->gibKaestchen(randX, randY);
            cout << "Zufällige Position: (" << randX << ", " << randY << ")" << endl;
            if (k.gibFarbe() == Gosu::Color::WHITE) {  // Prüfen, ob Kästchen weiß ist
                belegt = false;  // Position ist frei
                posX = randX;
                posY = randY;
                rückgabe = { randX, randY };
                menge++;  // Apfelmenge erhöhen
            }
        }
        return rückgabe;
    }
    int gibMenge() const {
        return menge;
    }
    int gibPosX()const {
        return posX;
    }
    int gibPosY()const {
        return posY;
    }
    void apfelMengedec() {
        menge = menge - 1;
        
    }
};

class Oberflaeche : public Gosu::Window {
private:
    Steuerung* steuerung; // Verweis auf Steuerung

public:
    // Konstruktor der Oberflaeche-Klasse
    Oberflaeche() : Gosu::Window(800, 600) {  // Fenstergröße 800x600
        set_caption("Snake");
        steuerung = new Steuerung(); // Steuerung instanziieren
    }

    ~Oberflaeche() {
        delete steuerung;  // Speicher für die Steuerung freigeben
    }

    // Zeichenfunktion, die in jedem Frame aufgerufen wird
    void zeichne() override {
        // Durch das 2D-Feld iterieren und die Kästchen zeichnen
        int max = steuerung->gibGroesseFeld();  // Zugriff auf Steuerung
        for (int i = 0; i < max; ++i) {
            for (int j = 0; j < max; ++j) {
                Kaestchen& k = steuerung->gibKaestchen(i, j);   // Kästchen-Objekt holen

                // Zeichne das Kästchen basierend auf seiner Position und Farbe
                int x = k.gibPosX();
                int y = k.gibPosY();
                Gosu::Graphics::draw_quad(  // Rechteck malen
                    x, y, k.gibFarbe(),                                // Oben links
                    x + 50, y, k.gibFarbe(),                           // Oben rechts
                    x + 50, y + 50, k.gibFarbe(),                      // Unten rechts
                    x, y + 50, k.gibFarbe(),                           // Unten links
                    0                                                  // Z-Ebene
                );

                // Zeichne die schwarzen Ränder
                Gosu::Graphics::draw_line(x, y, Gosu::Color::BLACK, x + 50, y, Gosu::Color::BLACK, 0);               // Oben
                Gosu::Graphics::draw_line(x, y + 50, Gosu::Color::BLACK, x + 50, y + 50, Gosu::Color::BLACK, 0);     // Unten
                Gosu::Graphics::draw_line(x, y, Gosu::Color::BLACK, x, y + 50, Gosu::Color::BLACK, 0);               // Links
                Gosu::Graphics::draw_line(x + 50, y, Gosu::Color::BLACK, x + 50, y + 50, Gosu::Color::BLACK, 0);     // Rechts
            }
        }
    }
};

int main() {
    Oberflaeche fenster;
    fenster.show();
}
