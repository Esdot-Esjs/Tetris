#include <QApplication>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QKeyEvent>
#include <QTimer>
#include <QPainter>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <fstream>
#include <vector>
#include <random>
#include <QMessageBox>

using namespace std;

//Klasa Gracza
class Gracz {
public:
    string nazwa;

    Gracz() {
        fstream daneGracza;
        daneGracza.open("dane_gracza.txt");
        if (daneGracza.is_open()) {
            daneGracza >> nazwa;
        }
        daneGracza.close();
    }
    void nowa_nazwa_gracza(const string nowaNazwa) {
        nazwa = nowaNazwa;
    }
};

// Logika tetrisa
class Tetris : public QWidget {
public:

    //Podstawowe zmienne
    string nazwaGracza;
    int szerokoscPlanszy = 10;
    int wysokoscPlanszy = 20;
    int rozmiarKlocka = 30;
    int szerokoscBocznegoPanelu = 180;

    struct Punkt { int x, y; };
    struct Tetromino { vector<Punkt> klocki; QColor kolor; };

    QTimer *zegar;
    int wynik;
    int wyczyszczoneLinie;
    bool czyKoniec;
    bool czyGra;

    vector<vector<int>> plansza;
    Tetromino aktualnyKlocek{};
    Tetromino nastepnyKlocek{};
    vector<Tetromino> klocki;
    int klocekX = 0, klocekY = 0;

    void InicjacjaKlockow() {
        // I
        klocki.push_back({{{0,0},{1,0},{2,0},{3,0}}, QColor(255,186,8)});
        // J
        klocki.push_back({{{0,0},{0,1},{1,1},{2,1}}, QColor(208,0,0)});
        // L
        klocki.push_back({{{2,0},{0,1},{1,1},{2,1}}, QColor(0,80,157)});
        // O
        klocki.push_back({{{1,0},{2,0},{1,1},{2,1}}, QColor(112,254,0)});
        // S
        klocki.push_back({{{1,0},{2,0},{0,1},{1,1}}, QColor(131,56 ,236)});
        // T
        klocki.push_back({{{1,0},{0,1},{1,1},{2,1}}, QColor(128,0,128)});
        // Z
        klocki.push_back({{{0,0},{1,0},{1,1},{2,1}}, QColor(0,255,0)});
    }

    Tetromino wylosujKlocek() {
        int index = rand() % klocki.size();
        return klocki[index];
    }

    bool sprawdzKolizje() {
        for (Punkt wspolrzedne : aktualnyKlocek.klocki) {
            int x = klocekX + wspolrzedne.x;
            int y = klocekY + wspolrzedne.y;
            if (x < 0 || x >= szerokoscPlanszy || y >= wysokoscPlanszy)
                return true;
            if (y >= 0 && plansza[y][x] != 0)
                return true;
        }
        return false;
    }

    void zatrzymajKlocek() {
        for (Punkt wspolrzedne : aktualnyKlocek.klocki) {
            int x = klocekX + wspolrzedne.x;
            int y = klocekY + wspolrzedne.y;
            if (y >= 0) {
                plansza[y][x] = aktualnyKlocek.kolor.rgba();
            }
        }
    }

    void wyczyscLinie() {
        int wyczyszczone = 0;
        for (int y = wysokoscPlanszy - 1; y >= 0; --y) {
            bool pelnaLinia = true;
            for (int x = 0; x < szerokoscPlanszy; ++x) {
                if (plansza[y][x] == 0) {
                    pelnaLinia = false;
                    break;
                }
            }
            if (pelnaLinia) {
                wyczyszczone++;
                for (int yy = y; yy > 0; --yy) {
                    plansza[yy] = plansza[yy - 1];
                }
                y++; // ponowne sprawdzenie przesuniętej linii
            }
        }
        if (wyczyszczone) {
            wynik += wyczyszczone * wyczyszczone * 10;
            wyczyszczoneLinie += wyczyszczone;
        }
    }

    void nowyKlocek() {
        aktualnyKlocek = nastepnyKlocek;
        nastepnyKlocek = wylosujKlocek();
        klocekX = szerokoscPlanszy / 2 - 2;
        klocekY = 0;
        if (sprawdzKolizje()) {
            koniecGry();
        }
    }

    void zaktualizujGre() {
        klocekY++;
        if (sprawdzKolizje()) {
            klocekY--;
            zatrzymajKlocek();
            wyczyscLinie();
            nowyKlocek();
        }
        update();
    }

    void koniecGry() {
        czyKoniec = true;
        czyGra = false;
        zegar->stop();
        ZapiszWynik();
        update();
        if (czyKoniec) {
            QMessageBox komunikat;
            komunikat.setWindowTitle("Koniec gry!");
            if (wynik == 0) {
                komunikat.setText("Twój wynik to: " + QString::number(wynik) + "\n Git gud");
            }else {
                komunikat.setText("Twój wynik to: " + QString::number(wynik));
            }
            //Przerobić by wracało do menu
            QPushButton *przyciskPowrot = komunikat.addButton("Wyjdź", QMessageBox::NoRole );
            komunikat.exec();

            if (komunikat.clickedButton() == przyciskPowrot) {
                QApplication::quit();
            }
        }
    }

    void ZapiszWynik() {
        ofstream wyniki("wyniki.txt", ios::app);
        if (wyniki.is_open()) {
            wyniki << nazwaGracza << " " << wynik << endl;
        }
    }

     Tetris(const string& Gracz, QWidget *parent = nullptr) : QWidget(parent) {
        nazwaGracza = Gracz;
        zegar = new QTimer(this);
        wynik = 0;
        wyczyszczoneLinie = 0;
        czyKoniec = false;
        czyGra = false;

        //Rozmiar okienka
        setFixedSize(szerokoscPlanszy * rozmiarKlocka + szerokoscBocznegoPanelu, wysokoscPlanszy * rozmiarKlocka);

        plansza.resize(wysokoscPlanszy, vector<int>(szerokoscPlanszy, 0));
        InicjacjaKlockow();

        QObject::connect(zegar, &QTimer::timeout, this, [this]() { zaktualizujGre(); });
    }

    void rozpocznij() {
        czyGra = true;
        czyKoniec = false;
        wynik = 0;
        wyczyszczoneLinie = 0;
        nastepnyKlocek = wylosujKlocek();
        nowyKlocek();
        zegar->start(500);
        update();
    }

    void ustawNazweGracza(const string& nowaNazwa) {
        nazwaGracza = nowaNazwa;
    }

    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);

        // tło planszy
        painter.setPen(Qt::black);
        painter.setBrush(Qt::black);
        painter.drawRect(0, 0, szerokoscPlanszy * rozmiarKlocka, wysokoscPlanszy * rozmiarKlocka);

        // użyte klocki
        for (int y = 0; y < wysokoscPlanszy; ++y) {
            for (int x = 0; x < szerokoscPlanszy; ++x) {
                if (plansza[y][x] != 0) {
                    painter.setBrush(QColor::fromRgba(plansza[y][x]));
                    painter.setPen(Qt::white);
                    painter.drawRect(x * rozmiarKlocka, y * rozmiarKlocka, rozmiarKlocka, rozmiarKlocka);
                }
            }
        }

        // aktualny klocek
        if (czyGra) {
            painter.setBrush(aktualnyKlocek.kolor);
            for (Punkt wspolrzedne : aktualnyKlocek.klocki) {
                int x = klocekX + wspolrzedne.x;
                int y = klocekY + wspolrzedne.y;
                if (y >= 0) {
                    painter.drawRect(x * rozmiarKlocka, y * rozmiarKlocka, rozmiarKlocka, rozmiarKlocka);
                    painter.setPen(Qt::white);
                    painter.drawRect(x * rozmiarKlocka, y * rozmiarKlocka, rozmiarKlocka, rozmiarKlocka);
                }
            }
        }

        // panel boczny
        int pozycjaStartowa = szerokoscPlanszy * rozmiarKlocka + 10;
        painter.setPen(Qt::black);
        painter.drawText(pozycjaStartowa, 30, "Gracz: " + QString::fromStdString(nazwaGracza));
        painter.drawText(pozycjaStartowa, 60, "Wynik: " + QString::number(wynik));
        painter.drawText(pozycjaStartowa, 90, "Linie: " + QString::number(wyczyszczoneLinie));
        painter.drawText(pozycjaStartowa, 160, "Następny:");
        for (Punkt wspolrzedne : nastepnyKlocek.klocki) {
            painter.setBrush(nastepnyKlocek.kolor);
            int nx = pozycjaStartowa + 20 + wspolrzedne.x * rozmiarKlocka;
            int ny = 180 + wspolrzedne.y * rozmiarKlocka;
            painter.drawRect(nx, ny, rozmiarKlocka, rozmiarKlocka);
            painter.setPen(Qt::white);
            painter.drawRect(nx, ny, rozmiarKlocka, rozmiarKlocka);
            painter.setPen(Qt::black);
        }
    }

    //Obsługa inputów z klawiatury
    void keyPressEvent(QKeyEvent *e) override {
        switch (e->key()) {
            case Qt::Key_Left:
                klocekX--;
                if (sprawdzKolizje()) {
                    klocekX++;
                }
                break;
            case Qt::Key_Right:
                klocekX++;
                if (sprawdzKolizje()) {
                    klocekX--;
                }
                break;
            case Qt::Key_Down:
                klocekY++;
                if (sprawdzKolizje()) {
                    klocekY--;
                }
                break;
            case Qt::Key_Up: { // obrót
                Punkt srodek = aktualnyKlocek.klocki[0];
                vector<Punkt> obrocone = aktualnyKlocek.klocki;
                for (int i = 0; i < obrocone.size(); ++i) {
                    int x = obrocone[i].x - srodek.x;
                    int y = obrocone[i].y - srodek.y;
                    obrocone[i].x = srodek.x - y;
                    obrocone[i].y = srodek.y + x;
                }
                vector<Punkt> poprzednie = aktualnyKlocek.klocki;
                aktualnyKlocek.klocki = obrocone;
                if (sprawdzKolizje()) {
                    aktualnyKlocek.klocki = poprzednie;
                }
                break;
            }
            case Qt::Key_Space: // drop
                while (!sprawdzKolizje()) {
                    klocekY++;
                }
                klocekY--;
                zatrzymajKlocek();
                wyczyscLinie();
                nowyKlocek();
                break;
            default:
                QWidget::keyPressEvent(e);
                return;
        }
        update();
    }
};

struct wyniki {
    string nazwa;
    int wynik;
};

// Interfejs
class Menu : public QWidget {
public:
    Menu(string &nazwa, QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Tetris");
        const QIcon icon("../icon_v2.png");
        QApplication::setWindowIcon(icon);

        QFont font("Lato", 16);
        QApplication::setFont(font);

        // Ustawienie głównego układu
        QVBoxLayout *glownyUklad = new QVBoxLayout(this);
        QStackedWidget *widok = new QStackedWidget(this);
        glownyUklad->addWidget(widok);
        setLayout(glownyUklad);

        //Menu główne
        QWidget *menuPrzyciski = new QWidget(this);
        QVBoxLayout *ukladMenu = new QVBoxLayout(menuPrzyciski);
        QLabel *Imie = new QLabel(QString("Witaj ") + nazwa.data() + QString("!"), menuPrzyciski);
        Imie->setAlignment(Qt::AlignHCenter);
        QPushButton *przyciskStart = new QPushButton("Start", menuPrzyciski);
        QPushButton *przyciskWyniki = new QPushButton("Tabela wyników", menuPrzyciski);
        QPushButton *przyciskZmianaGracza = new QPushButton("Zmiana gracza", menuPrzyciski);
        QPushButton *przyciskWyjscie = new QPushButton("Wyjście", menuPrzyciski);

        ukladMenu->addWidget(Imie);
        ukladMenu->addWidget(przyciskStart);
        ukladMenu->addWidget(przyciskWyniki);
        ukladMenu->addWidget(przyciskZmianaGracza);
        ukladMenu->addWidget(przyciskWyjscie);
        widok->addWidget(menuPrzyciski);

        //Gra
        Tetris *gra = new Tetris(nazwa, this);
        widok->addWidget(gra);

        //Tabela wyników
        QWidget *tabelaWynikowWidzet = new QWidget(this);
        QVBoxLayout *ukladTabeli = new QVBoxLayout(tabelaWynikowWidzet);
        QLabel *etykietaTabela = new QLabel("Wyniki", tabelaWynikowWidzet);
        etykietaTabela->setAlignment(Qt::AlignHCenter);
        QTableView *tabelaWidok = new QTableView(tabelaWynikowWidzet);
        QStandardItemModel *modelTabeli = new QStandardItemModel(0, 2, tabelaWidok);
        modelTabeli->setHeaderData(0, Qt::Horizontal, "Gracz");
        modelTabeli->setHeaderData(1, Qt::Horizontal, "Wynik");
        tabelaWidok->setModel(modelTabeli);
        tabelaWidok->horizontalHeader()->setStretchLastSection(true);
        tabelaWidok->verticalHeader()->setVisible(false);
        tabelaWidok->setSelectionMode(QAbstractItemView::NoSelection);
        tabelaWidok->setEditTriggers(QAbstractItemView::NoEditTriggers);

        QPushButton *przyciskPowrot = new QPushButton("Wróć", tabelaWynikowWidzet);
        ukladTabeli->addWidget(etykietaTabela);
        ukladTabeli->addWidget(tabelaWidok);
        ukladTabeli->addWidget(przyciskPowrot);
        widok->addWidget(tabelaWynikowWidzet);

        //Zmiana gracza
        QWidget *zmianaGraczaWidzet = new QWidget(this);
        QVBoxLayout *ukladZmiany = new QVBoxLayout(zmianaGraczaWidzet);
        QLabel *etykietaZmiana = new QLabel("Jak się nazywasz?", zmianaGraczaWidzet);
        etykietaZmiana->setAlignment(Qt::AlignHCenter);
        QLineEdit *poleNazwaGracza = new QLineEdit(zmianaGraczaWidzet);
        QPushButton *przyciskZapisz = new QPushButton("Zapisz", zmianaGraczaWidzet);
        ukladZmiany->addWidget(etykietaZmiana);
        ukladZmiany->addWidget(poleNazwaGracza);
        ukladZmiany->addWidget(przyciskZapisz);
        widok->addWidget(zmianaGraczaWidzet);


        //funkcja do wczytywania wyników z pliku
        auto wczytajWyniki = [=] {
            modelTabeli->removeRows(0, modelTabeli->rowCount()); // Wyczyść stare dane

            ifstream plikWyniki("wyniki.txt");
            if (!plikWyniki.is_open()) {
                return;
            }

            vector<wyniki> listaWynikow;
            string nazwaGracza;
            int wynik;
            while (plikWyniki >> nazwaGracza >> wynik) {
                listaWynikow.push_back({nazwaGracza, wynik});
            }

            // Sortowanie wyników
            sort(listaWynikow.begin(), listaWynikow.end(), [](wyniki& a, wyniki& b) {
                return a.wynik > b.wynik;
            });

            // Dodanie wyników do tabeli
            for (wyniki wpis : listaWynikow) {
                QList<QStandardItem*> wiersz;
                wiersz.append(new QStandardItem(QString::fromStdString(wpis.nazwa)));
                wiersz.append(new QStandardItem(QString::number(wpis.wynik)));
                modelTabeli->appendRow(wiersz);
            }
        };

        QObject::connect(przyciskStart, &QPushButton::clicked, [=]() {
            widok->setCurrentIndex(1);
            gra->rozpocznij();
            gra->setFocus();
        });
        QObject::connect(przyciskWyniki, &QPushButton::clicked, [=]() {
            wczytajWyniki();
            widok->setCurrentIndex(2);
        });
        QObject::connect(przyciskZmianaGracza, &QPushButton::clicked, [=]() {
            widok->setCurrentIndex(3);
        });
        QObject::connect(przyciskWyjscie, &QPushButton::clicked, [=]() {
            QApplication::quit();
        });
        QObject::connect(przyciskPowrot, &QPushButton::clicked, [=]() {
            widok->setCurrentIndex(0);
        });
        QObject::connect(przyciskZapisz, &QPushButton::clicked, [=]() {
            QString nowaNazwa = poleNazwaGracza->text();
            if (!nowaNazwa.isEmpty()) {
                string nowaNazwaStd = nowaNazwa.toStdString();
                fstream zapis;
                zapis.open("dane_gracza.txt", ios::out);
                if (zapis.is_open()) {
                    zapis << nowaNazwaStd;
                    zapis.close();
                    Imie->setText(QString("Witaj " + nowaNazwa + QString("!")));
                    gra->ustawNazweGracza(nowaNazwaStd);
                    widok->setCurrentIndex(0);
                }
            }
        });

        if (nazwa.empty()) {
            widok->setCurrentIndex(3);
        } else {
            widok->setCurrentIndex(0);
        }
    }
};

int main(int argc, char *argv[]) {
    srand(time(NULL));

    QApplication a(argc, argv);
    Gracz gracz;
    Menu menu(gracz.nazwa);
    menu.show();
    return QApplication::exec();
}