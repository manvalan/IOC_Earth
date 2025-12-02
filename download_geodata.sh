#!/bin/bash

# Script per scaricare dati geografici gratuiti per IOC_Earth

echo "========================================="
echo "IOC_Earth - Download Dati Geografici"
echo "========================================="
echo ""

# Directory di destinazione
DATA_DIR="$(cd "$(dirname "$0")" && pwd)/data"
mkdir -p "$DATA_DIR"

echo "Directory dati: $DATA_DIR"
echo ""

# Funzione per scaricare e estrarre
download_and_extract() {
    local url=$1
    local filename=$(basename "$url")
    local description=$2
    
    echo "📥 Download: $description"
    echo "   URL: $url"
    
    cd "$DATA_DIR"
    
    if [ -f "$filename" ]; then
        echo "   ⚠️  File già esistente, salto il download"
    else
        curl -L -o "$filename" "$url"
        if [ $? -ne 0 ]; then
            echo "   ❌ Errore nel download"
            return 1
        fi
        echo "   ✓ Download completato"
    fi
    
    # Estrai se è un archivio
    if [[ $filename == *.zip ]]; then
        echo "   📦 Estrazione in corso..."
        unzip -o "$filename"
        if [ $? -ne 0 ]; then
            echo "   ❌ Errore nell'estrazione"
            return 1
        fi
        echo "   ✓ Estrazione completata"
    fi
    
    echo ""
}

echo "Inizio download dei dati geografici..."
echo ""

# 1. Paesi del mondo (50m resolution - bassa risoluzione, più veloce)
download_and_extract \
    "https://naciscdn.org/naturalearth/50m/cultural/ne_50m_admin_0_countries.zip" \
    "Confini nazionali (50m - veloce)"

# 2. Coste e oceani
download_and_extract \
    "https://naciscdn.org/naturalearth/50m/physical/ne_50m_coastline.zip" \
    "Linee costiere"

# 3. Laghi
download_and_extract \
    "https://naciscdn.org/naturalearth/50m/physical/ne_50m_lakes.zip" \
    "Laghi"

# 4. Fiumi
download_and_extract \
    "https://naciscdn.org/naturalearth/50m/physical/ne_50m_rivers_lake_centerlines.zip" \
    "Fiumi"

echo "========================================="
echo "✓ Download completato!"
echo "========================================="
echo ""
echo "File scaricati in: $DATA_DIR"
echo ""
echo "Shapefile disponibili:"
ls -1 "$DATA_DIR"/*.shp 2>/dev/null | while read file; do
    echo "  - $(basename "$file")"
done
echo ""
echo "Ora puoi usare questi file nei tuoi programmi con:"
echo '  renderer.addShapefileLayer("data/ne_10m_admin_0_countries.shp", "countries");'
echo ""
