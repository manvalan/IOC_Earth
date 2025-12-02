# IOC_Earth

Libreria C++ per il rendering di mappe e tracciati GPS utilizzando Mapnik.

## 📋 Funzionalità

- ✅ Rendering di mappe in formato PNG
- ✅ Visualizzazione di tracciati GPS con timestamp
- ✅ Supporto per Shapefile come layer di base
- ✅ Etichettatura automatica dei punti con `text_symbolizer`
- ✅ Calcolo automatico dell'estensione della mappa dai punti GPS
- ✅ Personalizzazione di colori, stili e dimensioni

## 🚀 Quick Start

### Prerequisiti

- **CMake** >= 3.10
- **C++17** o superiore
- **Mapnik** >= 3.0
- **Boost** (system, filesystem)

### Installazione dipendenze

#### macOS
```bash
brew install mapnik boost cmake
```

#### Ubuntu/Debian
```bash
sudo apt-get install libmapnik-dev libboost-all-dev cmake
```

### Compilazione

```bash
# Clona il repository
git clone https://github.com/manvalan/IOC_Earth.git
cd IOC_Earth

# Crea la directory di build
mkdir build && cd build

# Configura con CMake
cmake ..

# Compila
make

# Installa (opzionale)
sudo make install
```

## 📖 Utilizzo

### Esempio base

```cpp
#include "MapPathRenderer.h"

int main() {
    // Crea un renderer 800x600
    ioc_earth::MapPathRenderer renderer(800, 600);
    
    // Imposta l'estensione geografica
    renderer.setExtent(11.0, 41.5, 13.0, 43.5);
    
    // Imposta il colore di sfondo
    renderer.setBackgroundColor("#E8F4F8");
    
    // Renderizza
    renderer.renderToFile("output.png");
    
    return 0;
}
```

### Esempio con tracciato GPS

```cpp
#include "MapPathRenderer.h"
#include <vector>

int main() {
    ioc_earth::MapPathRenderer renderer(1200, 800);
    
    // Definisci i punti GPS
    std::vector<ioc_earth::GPSPoint> points = {
        {12.4964, 41.9028, "08:00 Roma"},
        {11.2558, 43.7696, "13:00 Firenze"}
    };
    
    // Calcola automaticamente l'estensione
    renderer.autoSetExtentFromPoints(points, 15.0);
    
    // Aggiungi il tracciato (blu, spessore 3)
    renderer.addGPSPath(points, "#0066CC", 3.0);
    
    // Aggiungi etichette
    renderer.addPointLabels(points, "timestamp", 12);
    
    // Renderizza
    renderer.renderToFile("gps_track.png");
    
    return 0;
}
```

### Compilare i tuoi programmi

```bash
# Con pkg-config
g++ -std=c++17 mio_programma.cpp $(pkg-config --cflags --libs ioc_earth mapnik) -o mio_programma

# Con CMake
find_package(PkgConfig REQUIRED)
pkg_check_modules(IOC_EARTH REQUIRED ioc_earth)
target_link_libraries(mio_target ${IOC_EARTH_LIBRARIES})
```

## 🔧 API Reference

### Classe `MapPathRenderer`

#### Costruttore
```cpp
MapPathRenderer(unsigned int width, unsigned int height)
```
Crea un renderer con le dimensioni specificate.

#### Metodi principali

- **`setExtent(min_lon, min_lat, max_lon, max_lat)`**  
  Imposta l'estensione geografica della mappa.

- **`autoSetExtentFromPoints(points, margin_percent = 10.0)`**  
  Calcola automaticamente l'estensione basata sui punti GPS con un margine percentuale.

- **`addShapefileLayer(shapefile_path, layer_name)`**  
  Aggiunge uno shapefile come layer di base.

- **`addGPSPath(points, line_color = "blue", line_width = 2.0)`**  
  Aggiunge un tracciato GPS alla mappa.

- **`addPointLabels(points, label_field = "timestamp", font_size = 10)`**  
  Aggiunge etichette ai punti GPS.

- **`setBackgroundColor(color)`**  
  Imposta il colore di sfondo della mappa.

- **`renderToFile(output_path)`**  
  Renderizza la mappa in un file PNG.

### Struttura `GPSPoint`

```cpp
struct GPSPoint {
    double longitude;
    double latitude;
    std::string timestamp;
};
```

## 📁 Struttura del Progetto

```
IOC_Earth/
├── include/             # File header pubblici
│   └── MapPathRenderer.h
├── src/                 # Implementazione
│   └── MapPathRenderer.cpp
├── examples/            # Programmi di esempio
│   ├── simple_map.cpp
│   └── gps_track.cpp
├── data/                # Dati di esempio (shapefile, ecc.)
├── CMakeLists.txt       # Configurazione CMake
├── ioc_earth.pc.in      # Template pkg-config
└── README.md

```

## 🧪 Esempi

Il progetto include programmi di esempio nella directory `examples/`:

```bash
# Dopo la compilazione
cd build

# Esegui l'esempio semplice
./examples/simple_map

# Esegui l'esempio con tracciato GPS
./examples/gps_track
```

## 🤝 Contribuire

I contributi sono benvenuti! Per favore:

1. Fai un fork del progetto
2. Crea un branch per la tua feature (`git checkout -b feature/nuova-funzionalita`)
3. Committa le modifiche (`git commit -am 'Aggiungi nuova funzionalità'`)
4. Pusha il branch (`git push origin feature/nuova-funzionalita`)
5. Apri una Pull Request

## 📄 Licenza

Questo progetto è distribuito sotto licenza MIT. Vedi il file `LICENSE` per maggiori dettagli.

## 👤 Autore

**Michele Bigi** - [manvalan](https://github.com/manvalan)

## 🙏 Ringraziamenti

- [Mapnik](https://mapnik.org/) - Toolkit per il rendering di mappe
- Comunità open source per il supporto e le risorse
