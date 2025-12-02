# IOC_Earth

Libreria C++ per il rendering di mappe e tracciati GPS utilizzando Mapnik.

## 📋 Funzionalità

- ✅ Rendering di mappe in formato PNG
- ✅ Visualizzazione di tracciati GPS con timestamp
- ✅ Supporto per Shapefile come layer di base
- ✅ Etichettatura automatica dei punti con `text_symbolizer`
- ✅ Calcolo automatico dell'estensione della mappa dai punti GPS
- ✅ Personalizzazione di colori, stili e dimensioni
- ✅ **API specializzata per occultazioni asteroidali**
  - Visualizzazione del percorso centrale dell'ombra
  - Limiti di incertezza (1-sigma)
  - Time markers lungo il percorso
  - Stazioni di osservazione con stato
  - Caricamento dati da JSON compatibile con IOCalc

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

## 🌟 Visualizzazione Occultazioni Asteroidali

La libreria include un'API specializzata per visualizzare eventi di occultazione asteroidale:

```cpp
#include "OccultationRenderer.h"

// Crea il renderer (1600x1200 pixels)
ioc_earth::OccultationRenderer renderer(1600, 1200);

// Carica i dati da file JSON (formato compatibile IOCalc)
renderer.loadFromJSON("chariklo_occultation.json");

// Personalizza lo stile
ioc_earth::OccultationRenderer::RenderStyle style;
style.central_line_color = "#FF0000";        // Rosso per linea centrale
style.central_line_width = 3.0;
style.sigma_lines_color = "#FF8800";         // Arancione per limiti 1-sigma
style.sigma_lines_width = 2.0;
style.time_markers_color = "#0000FF";        // Blu per marker temporali
style.show_time_labels = true;
style.show_station_labels = true;

renderer.setRenderStyle(style);

// Renderizza con shapefile di sfondo
renderer.renderOccultationMap("occultation_map.png", true);

// OPPURE: Esporta direttamente in HTML con immagine embedded
renderer.exportToHTML("report.html", true, "Mappa Occultazione");

// OPPURE: Ottieni il buffer PNG per uso programmatico
std::vector<uint8_t> png_buffer;
renderer.renderToBuffer(png_buffer, true);
// Ora puoi inviare png_buffer via rete, salvarlo in DB, ecc.

// OPPURE: Ottieni l'immagine in formato base64
std::string base64_image = renderer.getLastRenderedImageBase64();
// Usa per embedding in JSON, HTML, ecc.
```

### Metodi Disponibili

#### `renderOccultationMap(output_path, include_shapefile)`
Renderizza la mappa e salva su file PNG.

#### `exportToHTML(output_html_path, include_shapefile, page_title)`
Genera una pagina HTML completa con:
- Mappa embedded in base64
- Informazioni sull'evento (asteroide, stella, data, durata, magnitudine)
- Legenda interattiva
- Design responsivo moderno

#### `renderToBuffer(png_data, include_shapefile)`
Renderizza la mappa e restituisce i dati PNG in un `std::vector<uint8_t>`.
Utile per:
- Invio via rete (HTTP API)
- Storage in database
- Processamento ulteriore

#### `getLastRenderedImageBase64()`
Restituisce l'ultima immagine renderizzata in formato base64.
Utile per:
- Embedding in HTML: `<img src="data:image/png;base64,...">`
- JSON API responses
- Database storage (campi TEXT/CLOB)
```

### Formato JSON

```json
{
  "id": "2024-06-03-Chariklo",
  "event_type": "asteroid_occultation",
  "asteroid": {
    "number": 10199,
    "name": "Chariklo",
    "diameter_km": 232.0
  },
  "star": {
    "catalog_id": "TYC 6009-01262-1",
    "photometry": {
      "magnitude_v": 11.3
    }
  },
  "event": {
    "date_time": {
      "iso8601": "2024-06-03T22:15:30Z"
    },
    "circumstances": {
      "duration_seconds": 8.5,
      "magnitude_drop": 5.2
    }
  },
  "shadow_path": {
    "central_line": [
      {"lon": -10.0, "lat": 38.5, "time": "22:13:00"},
      ...
    ],
    "northern_limit_1sigma": [...],
    "southern_limit_1sigma": [...]
  },
  "time_markers": [
    {"lon": -2.5, "lat": 41.2, "time": "22:15:30", "seconds_from_mid": 0},
    ...
  ],
  "observation_stations": [
    {"name": "Madrid", "lon": -3.7, "lat": 40.4, "status": "positive"},
    ...
  ]
}
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

# Esegui l'esempio con mappa dell'Italia
./examples/italy_map

# Esegui l'esempio di occultazione asteroidale (PNG)
./examples/occultation_map

# Esegui l'esempio di export HTML con buffer
./examples/occultation_html

# Esegui il demo completo di utilizzo API
./examples/api_usage_demo
```

### 🔌 Integrazione con Applicazioni

L'API è progettata per essere facilmente integrata in qualsiasi tipo di applicazione. Ecco alcuni esempi pratici:

#### REST API / Web Service

```cpp
// Endpoint HTTP che restituisce l'immagine PNG
std::vector<uint8_t> generateMapForAPI(const std::string& event_id) {
    ioc_earth::OccultationRenderer renderer(1600, 1200);
    renderer.loadFromJSON("data/" + event_id + ".json");
    
    std::vector<uint8_t> png_buffer;
    renderer.renderToBuffer(png_buffer, true);
    
    // Restituisci png_buffer come HTTP response
    // Content-Type: image/png
    return png_buffer;
}
```

#### JSON API

```cpp
// Endpoint che restituisce JSON con immagine embedded
std::string generateJSONResponse(const std::string& event_id) {
    ioc_earth::OccultationRenderer renderer(1600, 1200);
    renderer.loadFromJSON("data/" + event_id + ".json");
    
    std::vector<uint8_t> buffer;
    renderer.renderToBuffer(buffer, true);
    std::string base64 = renderer.getLastRenderedImageBase64();
    
    return "{ \"map\": \"data:image/png;base64," + base64 + "\" }";
}
```

#### Desktop Application (Qt)

```cpp
// Integrazione con Qt
QPixmap loadMapInQt(const std::string& json_path) {
    ioc_earth::OccultationRenderer renderer(1600, 1200);
    renderer.loadFromJSON(json_path);
    
    std::vector<uint8_t> png_buffer;
    renderer.renderToBuffer(png_buffer, true);
    
    QPixmap pixmap;
    pixmap.loadFromData(png_buffer.data(), png_buffer.size(), "PNG");
    return pixmap;
}
```

#### Mobile App (React Native / Flutter)

```javascript
// React Native
fetch('http://api.example.com/occultation/map?id=2024-06-03')
  .then(response => response.json())
  .then(data => {
    // data.map contiene l'immagine in base64
    return <Image source={{uri: data.map}} />;
  });

// Flutter
Image.memory(base64Decode(mapData))
```

#### Electron / Web Desktop App

```javascript
// Ottieni l'immagine dall'API C++ via binding
const pngBuffer = nativeModule.generateMap(eventId);
const blob = new Blob([pngBuffer], { type: 'image/png' });
const url = URL.createObjectURL(blob);
document.getElementById('map').src = url;
```
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
