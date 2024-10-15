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

//gewonnen fehlt

using namespace std;
using namespace Gosu;

class Steuerung;

class Schlange {
private:
    vector<tuple<int, int, int>> segmente;  // Die Schlange als Liste von Segmenten (Stelle der Schlange, x, y Kästchen)
    int richtungX = 1; //-1 nach oben, 1 nach unten
    int richtungY = 0;  // 1 nach rechts, -1 nach links
    Color farbe;
    bool richtungAndernErlaubt = true;      //blockiert mehrfache eingaben
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
    bool gibRichtungAndernErlaubt() { return richtungAndernErlaubt; }
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
    int highscore = 0;

    Kaestchen kaestchen[10][10];
    Apfel* apfel;
    Schlange* schlange;
    Sample* snakeEating;

public:
    Steuerung();
    ~Steuerung();
    int gibGroesseFeld() const { return rasterBreite; }
    Kaestchen& gibKaestchen(int i, int j);
    int gibApfelPosX();
    int gibApfelPosY();
    bool gibRichtungAndernErlaubt();//gibt den Wert von Schlange weiter
    void apfelEntfernen();
    void apfelPlatzieren();
    void apfelGegessen(int posX, int posY);
    void kollisionMitWand(int kopfX, int kopfY);
    void kollisionMitSichSelbst(int kopfX, int kopfY);
    void verloren();
    void neustart();
    int gibSpielstand() { return spielstand; }
    double gibaktualisierungsZeit() { return aktualisierungsZeit; }
    int gibGroessseSchlange() { return schlange->gibGroesse(); }
    void setzteAktualisierungsZeit(double aktualisierungsZeit) { this->aktualisierungsZeit = aktualisierungsZeit; }
    int gibHighscore() { return highscore; }
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
    int neuerKopfX = get<1>(segmente.front()) + richtungX;//holt die X position des Kopfes
    int neuerKopfY = get<2>(segmente.front()) + richtungY;//holt die Y position des Kopfes

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
    richtungAndernErlaubt = true;   //Tasteneingaben wieder erlauben
}

void Schlange::setzeRichtung(int x, int y) {        //x,y neue richtung
    // Verhindert das Umdrehen auf sich selbst
    if ((richtungX != -x || richtungX == 0) && (richtungY != -y || richtungY == 0)) {
        richtungX = x;
        richtungY = y;
        richtungAndernErlaubt = false;  //nachdem eine gültige eingabe getätigt wurde wird die eingabe gesperrt bis die Schlange im nächsten Feld ist
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
    
    snakeEating = new Gosu::Sample("snakeEating.wav");
    snakeEating->play();
    cout << "sound ausgegeben";
    

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
    delete snakeEating;
}

void Steuerung::verloren() {
    setzteAktualisierungsZeit(10000); //10 minuten 
    spielstand = 2;
    if ((schlange->gibGroesse() + 1) > highscore) {
        highscore = schlange->gibGroesse();
    }

}

void Steuerung::neustart() {
    // Spielfeld zurücksetzen
    for (int i = 0; i < rasterHoehe; ++i) {
        for (int j = 0; j < rasterBreite; ++j) {
            kaestchen[i][j].setzeFarbe(Color::WHITE);

            // Setze die Ränder des Spielfelds auf grau
            if (i == 0 || j == 0 || i == rasterHoehe - 1 || j == rasterBreite - 1) {
                kaestchen[i][j].setzeFarbe(Color::GRAY);
            }
        }
    }

    // Schlange zurücksetzen
    delete schlange; // Alte Schlange löschen
    schlange = new Schlange(this); // Neue Schlange initialisieren

    // Apfel neu platzieren
    apfelEntfernen();  // Apfel vom alten Ort entfernen
    apfelPlatzieren(); // Apfel an eine neue zufällige Position setzen

    // Spielstand auf "spielen" setzen
    spielstand = 1;

    // Aktualisierungszeit zurücksetzen (langsamer Start)
    setzteAktualisierungsZeit(0.5); // Längere Zeit für den Start, damit Spieler die Schlange steuern kann
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
bool Steuerung::gibRichtungAndernErlaubt() {
    return schlange->gibRichtungAndernErlaubt();
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
    Image* neustartKnopf;   //Zeiger auf das Neustartknopf-Bild
    Image* snakekopf;
    Image* apple;
    string nachricht = "Verloren \nGröße: 12 \nHighscore: 00";
    double skalierungNeustart = 0.15;
    //Breite der Nachricht
    double textBreite;
    double textHoehe;
    // Positionierung des Textes in der Mitte des Bildschirms
    double x;
    double y;
    double winkel;
public:
    Oberflaeche() : Window(800, 600), font(30), last_move_time(0), winkel(0.0) {
        neustartKnopf = new Image("neustartKnopf.png");// Zeiger auf das Neustartknopf-Bild
        snakekopf = new Image("schlangenkopf.png");
        apple = new Image("apple.png");
        

        set_caption("Snake");
        steuerung = new Steuerung();

        //Höhe und breite berechnen der Nachricht
        textBreite = font.text_width(nachricht);
        textHoehe = font.height() * 2;

        // Positionierung des Textes in der Mitte des Bildschirms
        x = 400 - textBreite / 2;
        y = 300 - textHoehe / 2;
    }

    ~Oberflaeche() {
        delete steuerung;
        delete neustartKnopf;  // Speicher für das Bild freigeben
        delete snakekopf;
        delete apple;
        
    }


    void update() override {
        double current_time = milliseconds() / 1000.0;

        if (Gosu::Input::down(Gosu::MS_LEFT)) {  // Linksklick auf neustart Feld
            int mausX = input().mouse_x();
            int mausY = input().mouse_y();

            // Berechne die Koordinaten und Dimensionen des Rechtecks
            int rechteckObenLinksX = x - 12;
            int rechteckObenLinksY = y - 12;
            int rechteckUntenRechtsX = x + textBreite + 12;
            int rechteckUntenRechtsY = y + textHoehe + 40 + (neustartKnopf->height() * skalierungNeustart);

            // Überprüfen, ob die Maus innerhalb des Rechtecks geklickt wurde
            if (mausX >= rechteckObenLinksX && mausX <= rechteckUntenRechtsX &&
                mausY >= rechteckObenLinksY && mausY <= rechteckUntenRechtsY &&
                steuerung->gibSpielstand() == 2) {
                steuerung->neustart();
            }
        }
        if (current_time - last_move_time >= steuerung->gibaktualisierungsZeit()) {  // Bewegung wenn der milliseconds auf den wert der auf dem Wert der aktualisierungsZeit ist
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
                            if (get<0>(segment) == 0) {  // Kopfsegment an 0. Stelle wird geprüft
                                farbe = Color::WHITE;
                                // Abrufen der Richtung
                                int richtungX = steuerung->gibSchlange()->gibRichtungX();
                                int richtungY = steuerung->gibSchlange()->gibRichtungY();

                                // Winkel bestimmen basierend auf der Richtung
                                double drehwinkel = 0.0;
                                if (richtungX == -1 && richtungY == 0) {  // nach oben
                                    drehwinkel = 180.0;

                                }
                                else if (richtungX == 1 && richtungY == 0) {  // nach unten
                                    drehwinkel = 0.0;
                                }
                                else if (richtungX == 0 && richtungY == -1) {  // nach links
                                    drehwinkel = 90.0;
                                }
                                else if (richtungX == 0 && richtungY == 1) {  // nach rechts

                                    drehwinkel = 270.0;
                                }

                                // Kopf der Schlange rotieren und zeichnen
                                snakekopf->draw_rot(k.gibPosX() + 25, k.gibPosY() + 25, 1, drehwinkel, 0.5, 0.5, 0.15, 0.15);
                            }
                            break;
                        }
                    }
                    if (Color::RED == farbe) {
                        farbe = Color::WHITE;
                        apple->draw(x - 2, y - 4, 1, 0.15, 0.15);  // Apfel-Bild zeichnen
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
            nachricht = "Verloren \nGröße: " + to_string(steuerung->gibGroessseSchlange()) + "\nHighscore: " + to_string(steuerung->gibHighscore()+1);

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
            // Zeichne einen weißen Rand um den Kasten
            Graphics::draw_quad(
                x - 12, y - 12, randFarbe,          // Oben links
                x + textBreite + 12, y - 12, randFarbe,  // Oben rechts
                x + textBreite + 12, y + textHoehe + 40 + (neustartKnopf->height() * skalierungNeustart), randFarbe,  // Unten rechts
                x - 12, y + textHoehe + 40 + (neustartKnopf->height() * skalierungNeustart), randFarbe,  // Unten links
                1  // Z-Ebene des Randes (über dem Hintergrund, aber unter dem Text)
            );

            // Zeichne den Text über den Kasten
            font.draw_text(nachricht, x, y, 2, 1.0, 1.0, Color::RED);
            neustartKnopf->draw(x + 20, y + 95, 1, skalierungNeustart, skalierungNeustart);  // Position (x, y), Z-Ebene 1, Skalierung 0.5x
        }
    }

    void button_down(Button btn) override { //Knopfdruck zum steuern der schlange
        switch (btn) {
        case KB_W:  // nach oben
            if (steuerung->gibRichtungAndernErlaubt() == true) {
                steuerung->gibSchlange()->setzeRichtung(-1, 0);
            }
            break;
        case KB_S:  // nach unten
            if (steuerung->gibRichtungAndernErlaubt() == true) {
                steuerung->gibSchlange()->setzeRichtung(1, 0);
            }
            break;
        case KB_A:  // nach links
            if (steuerung->gibRichtungAndernErlaubt() == true) {
                steuerung->gibSchlange()->setzeRichtung(0, -1);
            }
            break;
        case KB_D:  // nach rechts
            if (steuerung->gibRichtungAndernErlaubt() == true) {
                steuerung->gibSchlange()->setzeRichtung(0, 1);
            }
            break;
        case KB_P:
            cout << "Pause";
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