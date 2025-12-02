# Note sulla directory data/

Questa directory è destinata a contenere dati geografici come:

- **Shapefile** (`.shp`, `.shx`, `.dbf`, `.prj`)
- **File GeoJSON**
- **Dati raster** (`.tif`, `.png`)
- **Altri formati supportati da Mapnik**

## Dove trovare dati geografici

### Shapefile gratuiti

1. **Natural Earth**
   - URL: https://www.naturalearthdata.com/
   - Dati geografici globali di alta qualità

2. **GADM**
   - URL: https://gadm.org/
   - Confini amministrativi per tutti i paesi

3. **OpenStreetMap**
   - URL: https://www.openstreetmap.org/
   - Dati estratti tramite tools come osm2pgsql

4. **ISTAT (Italia)**
   - URL: https://www.istat.it/
   - Confini amministrativi italiani

### Esempio di utilizzo

```cpp
// Aggiungi uno shapefile alla mappa
renderer.addShapefileLayer("data/italy_borders.shp", "italy");
```

## Nota

I file di dati non sono inclusi nel repository per questioni di dimensione.
Scarica i dati necessari dai link sopra indicati.
