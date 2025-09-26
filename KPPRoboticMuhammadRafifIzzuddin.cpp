#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <cmath>
using namespace std;

class Jalan {
public:
    int tujuan;
    int jarak;
    int ongkos;

    Jalan(int t, int j, int o) {
        tujuan = t;
        jarak = j;
        ongkos = o;
    }
};

class Graf {
public:
    int jumlahSimpul;
    vector<vector<Jalan>> daftarAdj;

    Graf(int n) {
        jumlahSimpul = n;
        daftarAdj.resize(n + 1);
    }

    void tambahJalan(int dari, int ke, int jarak, int ongkos) {
        daftarAdj[dari].push_back(Jalan(ke, jarak, ongkos));
        daftarAdj[ke].push_back(Jalan(dari, jarak, ongkos));
    }
};

class Keadaan {
public:
    int simpul;
    int menit;
    double energiTerpakai;
    double energiTersisa;

    Keadaan(int s, int m, double eTerpakai, double eTersisa) {
        simpul = s;
        menit = m;
        energiTerpakai = eTerpakai;
        energiTersisa = eTersisa;
    }
};

class Banding {
public:
    bool operator()(Keadaan a, Keadaan b) {
        return a.energiTerpakai > b.energiTerpakai;
    }
};

class Robot {
public:
    double energiMaksimum;
    double energiAwal;
    int jamMulai;

    Robot(double eMaks, int jMulai) {
        energiMaksimum = eMaks;
        energiAwal = eMaks;
        jamMulai = jMulai;
    }

    double hitungFaktor(int menitSekarang) {
        int jam = jamMulai + (menitSekarang / 60);
        if (jam % 2 == 1) {
            return 1.3; 
        } else {
            return 0.8; 
        }
    }
};

int main() {
    int banyakSimpul, banyakJalan;
    cin >> banyakSimpul >> banyakJalan;

    map<string, int> namaKeId;
    map<int, string> idKeNama;
    int idSekarang = 1;

    auto getId = [&](string nama) {
        if (namaKeId.find(nama) == namaKeId.end()) {
            namaKeId[nama] = idSekarang;
            idKeNama[idSekarang] = nama;
            idSekarang++;
        }
        return namaKeId[nama];
    };

    Graf graf(banyakSimpul);

    for (int i = 0; i < banyakJalan; i++) {
        string a, b;
        int jarak, ongkos;
        cin >> a >> b >> jarak >> ongkos;
        int idA = getId(a);
        int idB = getId(b);
        graf.tambahJalan(idA, idB, jarak, ongkos);
    }

    string namaAwal, namaTujuan;
    cin >> namaAwal >> namaTujuan;
    int simpulAwal = getId(namaAwal);
    int simpulTujuan = getId(namaTujuan);

    string daftarRest, daftarCharging, simpulM, simpulE;
    cin >> daftarRest >> daftarCharging >> simpulM >> simpulE;

    int jamMulai;
    cin >> jamMulai;

    vector<bool> adalahRest(idSekarang, false);
    vector<bool> adalahCharging(idSekarang, false);

    if (daftarRest != "-") {
        adalahRest[getId(daftarRest)] = true;
    }
    if (daftarCharging != "-") {
        adalahCharging[getId(daftarCharging)] = true;
    }


    Robot robot(1000.0, jamMulai);


    priority_queue<Keadaan, vector<Keadaan>, Banding> antrian;


    map<pair<int,int>, double> energiTersimpan;
    map<pair<int,int>, int> parent;


    Keadaan awal(simpulAwal, 0, 0.0, robot.energiAwal);
    antrian.push(awal);
    energiTersimpan[{simpulAwal, 0}] = 0.0;
    parent[{simpulAwal, 0}] = -1;

    bool ketemu = false;
    int menitTujuan = -1;
    double energiMinimum = 1e18;

    while (!antrian.empty()) {
        Keadaan sekarang = antrian.top();
        antrian.pop();

        if (sekarang.simpul == simpulTujuan) {
            ketemu = true;
            energiMinimum = sekarang.energiTerpakai;
            menitTujuan = sekarang.menit;
            break;
        }


        for (auto jalan : graf.daftarAdj[sekarang.simpul]) {
            double dasar = jalan.jarak + jalan.ongkos;
            double faktor = robot.hitungFaktor(sekarang.menit);
            double butuhEnergi = dasar * faktor;

            if (sekarang.energiTersisa >= butuhEnergi) {
                double sisaEnergi = sekarang.energiTersisa - butuhEnergi;
                double energiBaru = sekarang.energiTerpakai + butuhEnergi;
                int menitBaru = sekarang.menit + 2;

                if (adalahCharging[jalan.tujuan]) {
                    sisaEnergi = robot.energiMaksimum;
                }

                if (!energiTersimpan.count({jalan.tujuan, menitBaru}) || energiBaru < energiTersimpan[{jalan.tujuan, menitBaru}]) {
                    energiTersimpan[{jalan.tujuan, menitBaru}] = energiBaru;
                    parent[{jalan.tujuan, menitBaru}] = sekarang.simpul;
                    antrian.push(Keadaan(jalan.tujuan, menitBaru, energiBaru, sisaEnergi));
                }
            }
        }


        if (adalahRest[sekarang.simpul]) {
            int menitBaru = sekarang.menit + 1;
            if (!energiTersimpan.count({sekarang.simpul, menitBaru}) || sekarang.energiTerpakai < energiTersimpan[{sekarang.simpul, menitBaru}]) {
                energiTersimpan[{sekarang.simpul, menitBaru}] = sekarang.energiTerpakai;
                parent[{sekarang.simpul, menitBaru}] = sekarang.simpul;
                antrian.push(Keadaan(sekarang.simpul, menitBaru, sekarang.energiTerpakai, sekarang.energiTersisa));
            }
        }
    }

    if (!ketemu) {
        cout << "Robot gagal mencapai tujuan\n";
    } else {
        cout << "Total energi minimum: " << (int)round(energiMinimum) << "\n";
        cout << "Waktu tiba di simpul tujuan: " << menitTujuan << " menit\n";
    }

    return 0;
}