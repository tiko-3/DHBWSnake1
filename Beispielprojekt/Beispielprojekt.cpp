#include <Gosu/Gosu.hpp>
#include <Gosu/AutoLink.hpp>
#include <cstdlib>  // Für rand() und srand()
#include <ctime>    // Für time()
#include <iostream>
#include <cmath>
#include <array>
#include <vector>
#include <tuple>
#include <string>

using namespace std;
using namespace Gosu;

class Steuerung;

class Schlange {
private:
    vector<tuple<int, int, int>> segmente;  // Die Schlange als Liste von Segmenten (Stelle der Schlange, x, y Kästchen)
    int richtungX = 1; //-1 nach oben, 1 nach unten
    int richtungY = 0;  // 1 nach rechts, -1 nach links
    Color farbe;
    Steuerung* steuerung;

public:
    Schlange(Steuerung* steuerung);
    Color gibFarbe() const { return farbe; }
    void bewegen();
    void setzeRichtung(int x, int y);
    bool isstApfel(int apfelX, int apfelY);
    tuple<int, int, int> gibKopf() const { return segmente.front(); }
    const vector<tuple<int, int, int>>& gibSegmente() const { return segmente; }
    int gibRichtungX() { return richtungX; }
    int gibRichtungY() { return richtungY; }
    int gibGroesse() { return segmente.size(); }
};

class Kaestchen {
private:
    int posX;
    int posY;
    Color farbe;

public:
    Kaestchen(int x = 0, int y = 0, Color farbe = Color::WHITE)
        : posX(x), posY(y), farbe(farbe) {}

    int gibPosX() const { return posX; }
    int gibPosY() const { return posY; }
    Color gibFarbe() const { return farbe; }
    void setzePosition(int x, int y) { posX = x; posY = y; }
    void setzeFarbe(Color neueFarbe) { farbe = neueFarbe; }
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
    int spielstand = 1;     //0=start, 1=spielen, 2=verloren

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
    bool bewegenErlaubt(int richtung);
    void verloren();
    int gibSpielstand() { return spielstand; }
    double gibaktualisierungsZeit() { return aktualisierungsZeit; }
    int gibGroessseSchlange() { return schlange->gibGroesse(); }
    void setzteAktualisierungsZeit(int aktualisierungsZeit) { this->aktualisierungsZeit = aktualisierungsZeit; }
    Schlange* gibSchlange() { return schlange; }
};

// Implementierung der Schlange
Schlange::Schlange(Steuerung* steuerung) : steuerung(steuerung) {
    // Schlange startet auf Y-Koordinate 4
    segmente.push_back(make_tuple(0, 4, 4));  // Startposition in der Mitte des Rasters
    segmente.push_back(make_tuple(1, 3, 4));  // körper hinzufügen
    segmente.push_back(make_tuple(1, 2, 4));
    farbe = Color::GREEN;
}

void Schlange::bewegen() {
    int neuerKopfX = get<1>(segmente.front()) + richtungX;
    int neuerKopfY = get<2>(segmente.front()) + richtungY;

    if (isstApfel(steuerung->gibApfelPosX(), steuerung->gibApfelPosY())) {  //ob der Apfel gegessen wurde
        steuerung->apfelGegessen(neuerKopfX, neuerKopfY);
    }
    else {
        segmente.pop_back();
    }
    steuerung->kollisionMitWand(neuerKopfX, neuerKopfY);
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
    int max = steuerung->gibGroesseFeld() - 2;//-2 weil bei -1 sind wir auf dem letzten max feld, weil wir bei 0 anfangen mit zählen
    bool belegt = true;
    int randX = 4;  // Apfel startet auf der gleichen x-Koordinate wie die Schlange
    int randY = 0;
    std::array<int, 2> rueckgabe = { 0, 0 };

    while (belegt) {
        randX = (rand() % max) + 1;  // neue zufällige Position
        randY = (rand() % max) + 1;
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
    kaestchen[apfel->gibPosX()][apfel->gibPosY()].setzeFarbe(Color::WHITE);

}

void Steuerung::kollisionMitWand(int kopfX, int kopfY) {
    if (kopfX == 0) { //raus oben
        verloren();
    }
    if (kopfY == 0) {  //raus links
        verloren();
    }
    if (kopfX == rasterBreite - 1) { //raus unten
        verloren();
    }
    if (kopfY == rasterHoehe - 1) {   //raus rechts
        verloren();
    }
}

bool Steuerung::bewegenErlaubt(int richtung) {
    //hoch=0,runter=1, links=2, rechts=3
    //position des Kopfes holen und in ein automatische erstellter vektor
    auto erstesElement = schlange->gibSegmente().front(); 
    int x1, y1, z1;
    // einzelnen Elemente eines tuple auf separate Variablen zu verteilen, anstatt sie manuell mit get<index>(...) aus dem Tupel zu extrahieren.
    tie(x1, y1, z1) = erstesElement; //extrahiert die Werte aus dem tuple
    std::cout << "x: " << x1 << ", y: " << y1 << ", z: " << z1 << std::endl;

   auto zweitesElement = schlange->gibSegmente()[1];  // Zugriff auf das zweite Element
   int x2, y2, z2;
   tie(x2, y2, z2) = zweitesElement;

    //std::cout << "x2: " << x2 << ", y2: " << y2 << ", z2: " << z2 << std::endl;

    if (x1==(x2-1)&&y1==y2) {        //zweites element ist rechts von kopf||gleiche Zeile
        if (richtung==3) {  //verboten nach rechts zu gehen
            return false;
        }
    }
    if (x1 == (x2 + 1) && y1 == y2) {  //zweites element ist links von Kopf||beide auf gleiche Zeile
        if (richtung == 2) {    //verboten nach links zu gehen
            return false;
        }
    }
    if (x1 == x2 && y1 == (y2 - 1)) {// gleiche Spalte und Kopf ist über zweitem Element
        if (richtung==1) {    //verboten anch unten zu gehen
            return false;
        }
     }
    if (x1 == x2 && y1 == (y2 + 1)) {
        if (richtung == 0) {
            return false;
        }
    }
    return true;
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
    // Berechne den Offset, um das Spielfeld zu zentrieren
    int spielfeldBreite = rasterBreite * kaestchenGroesse;
    int spielfeldHoehe = rasterHoehe * kaestchenGroesse;

    // Fenstergröße (angenommen, das Fenster ist 800x600, du musst dies anpassen, wenn es anders ist)
    int fensterBreite = 800; // Beispiel: Breite des Fensters
    int fensterHoehe = 600;  // Beispiel: Höhe des Fensters

    // Offset zum Zentrieren des Spielfelds
    int offsetX = (fensterBreite - spielfeldBreite) / 2;
    int offsetY = (fensterHoehe - spielfeldHoehe) / 2;

    // Initialisiere die Kästchen mit zentrierten Positionen
    //links oben ist [0][0]
    for (int i = 0; i < rasterHoehe; ++i) {
        for (int j = 0; j < rasterBreite; ++j) {
            // Position jedes Kästchens berechnen und den Offset hinzufügen
            kaestchen[i][j].setzePosition(offsetX + j * kaestchenGroesse, offsetY + i * kaestchenGroesse);
            kaestchen[i][j].setzeFarbe(Color::WHITE);

            // Setze die Ränder des Spielfelds auf grau
            if (i == 0 || j == 0 || i == rasterHoehe - 1 || j == rasterBreite - 1) {
                kaestchen[i][j].setzeFarbe(Color::GRAY);
            }
        }
    }

    // Apfel und Schlange initialisieren
    apfel = new Apfel(this);
    schlange = new Schlange(this);

    // Apfel platzieren
    apfelPlatzieren();
}


Steuerung::~Steuerung() {
    delete apfel;
    delete schlange;
}

void Steuerung::verloren() {
    setzteAktualisierungsZeit(10000); //10 minuten 
    spielstand = 2;

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
    kaestchen[position[0]][position[1]].setzeFarbe(Color::RED);
}

void Steuerung::apfelGegessen(int posX, int posY) {
    if (kaestchen[apfel->gibPosX()][apfel->gibPosY()].gibFarbe() == Color::RED) {
        apfel->apfelMengedec();//Apfel menge wird um eins verringert (wenn es mehrere Äpfel gibt)
        apfelEntfernen();
        apfelPlatzieren();
    }
}

class Oberflaeche : public Window {
private:
    Steuerung* steuerung;
    double last_move_time;
    Font font; // Zum Darstellen von Text
public:
    Oberflaeche() : Window(800, 600), font(30), last_move_time(0) {
        set_caption("Snake");
        steuerung = new Steuerung();
    }

    ~Oberflaeche() {
        delete steuerung;
    }

    void update() override {
        double current_time = milliseconds() / 1000.0;
        if (current_time - last_move_time >= steuerung->gibaktualisierungsZeit()) {  // Bewegung alle 0,5 Sekunden
            steuerung->gibSchlange()->bewegen();
            last_move_time = current_time;
        }
    }

    void draw() override {


        if (steuerung->gibSpielstand() == 0) {

        }

        if (steuerung->gibSpielstand() == 1 || steuerung->gibSpielstand() == 2) {
            int max = steuerung->gibGroesseFeld();
            for (int i = 0; i < max; ++i) {
                for (int j = 0; j < max; ++j) {
                    Kaestchen& k = steuerung->gibKaestchen(i, j);
                    int x = k.gibPosX();
                    int y = k.gibPosY();

                    Color farbe = k.gibFarbe();

                    const vector<tuple<int, int, int>>& segmente = steuerung->gibSchlange()->gibSegmente();
                    for (const auto& segment : segmente) {
                        if (get<1>(segment) == i && get<2>(segment) == j) {
                            farbe = Color::GREEN;
                            break;
                        }
                    }

                    Graphics::draw_quad(
                        x, y, farbe,
                        x + 50, y, farbe,
                        x + 50, y + 50, farbe,
                        x, y + 50, farbe,
                        0
                    );

                    Graphics::draw_line(x, y, Color::BLACK, x + 50, y, Color::BLACK, 0);
                    Graphics::draw_line(x, y + 50, Color::BLACK, x + 50, y + 50, Color::BLACK, 0);
                    Graphics::draw_line(x, y, Color::BLACK, x, y + 50, Color::BLACK, 0);
                    Graphics::draw_line(x + 50, y, Color::BLACK, x + 50, y + 50, Color::BLACK, 0);
                }
            }
        }
        if (steuerung->gibSpielstand() == 2) {//verloren
            // Berechnung der Textgröße
            std::string nachricht = "Verloren \nGröße: " + std::to_string(steuerung->gibGroessseSchlange());

            double textBreite = font.text_width(nachricht);
            double textHoehe = font.height()*2;

            // Positionierung des Textes in der Mitte des Bildschirms
            double x = 400 - textBreite / 2;
            double y = 300 - textHoehe / 2;

            // Zeichne einen Kasten als Hintergrund für den Text
            Color kastenFarbe = Color::BLACK;
            Color randFarbe = Color::GRAY;

            // Hintergrundrechteck zeichnen
            Graphics::draw_quad(
                x - 10, y - 10, kastenFarbe,          // Oben links
                x + textBreite + 10, y - 10, kastenFarbe,  // Oben rechts
                x + textBreite + 10, y + textHoehe + 10, kastenFarbe,  // Unten rechts
                x - 10, y + textHoehe + 10, kastenFarbe,  // Unten links
                0  // Z-Ebene des Hintergrunds (unter dem Text)
            );

            // Zeichne einen weißen Rand um den Kasten (optional)
            Graphics::draw_quad(
                x - 12, y - 12, randFarbe,          // Oben links
                x + textBreite + 12, y - 12, randFarbe,  // Oben rechts
                x + textBreite + 12, y + textHoehe + 12, randFarbe,  // Unten rechts
                x - 12, y + textHoehe + 12, randFarbe,  // Unten links
                1  // Z-Ebene des Randes (über dem Hintergrund, aber unter dem Text)
            );

            // Zeichne den Text über den Kasten
            font.draw_text(nachricht, x, y, 2, 1.0, 1.0, Color::RED);
        }
    }

    void button_down(Button btn) override {
        switch (btn) {
        case KB_W:  // nach oben
            if (steuerung->bewegenErlaubt(0) == true) {      //hoch=0
                steuerung->gibSchlange()->setzeRichtung(-1, 0);
            }
            break;
        case KB_S:  // nach unten
            //runter 1
            if (steuerung->bewegenErlaubt(0) == true) {
                steuerung->gibSchlange()->setzeRichtung(1, 0);
            }
            break;
        case KB_A:  // nach links
            //links=2
            if (steuerung->bewegenErlaubt(0) == true) {
                steuerung->gibSchlange()->setzeRichtung(0, -1);
            }
            break;
        case KB_D:  // nach rechts
            //rechts=3
            if (steuerung->bewegenErlaubt(0) == true) {
                steuerung->gibSchlange()->setzeRichtung(0, 1);
            }
            break;
        case KB_P:
            if (steuerung->bewegenErlaubt(0) == true) {
                cout << "Pause";
            }
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