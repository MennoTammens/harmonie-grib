# Conversie GRIB-files HARMONIE-model

Het KNMI stelt in het [KNMI Data Platform](https://dataplatform.knmi.nl/catalog/index.html) GRIB-files van het Harmonie-model vrij beschikbaar als [Open Data](http://creativecommons.org/publicdomain/mark/1.0/deed.nl). Het HARMONIE-AROME Cy40-model levert data tot 48 uur vooruit met een hoge resolutie (2,5×2,5km) en in tijdstappen van 1 uur.
De GRIB-files van het KNMI zijn nog niet direct in GRIB-viewers als [XyGrib](https://opengribs.org/en/xygrib) te openen omdat het formaat een beetje afwijkt van wat de GRIB-viewers verwachten. Ik heb daarom de GRIB-files geconverteerd in een formaat dat wel te gebruiken is in XyGrib en diverse andere GRIB-viewers

Voor meer informatie en discussie, zie dit topic op Zeilersforum: https://zeilersforum.nl/index.php/forum-125/meteo/575942-hoge-resolutie-grib-files

## Usage
- From Docker Hub:

  ```
  mkdir -p data
  docker run --rm --name grib-download -e KNMI_API_KEY=1234567890abcdef -e METEOGRAM_LAT=51.2345 -e METEOGRAM_LON=6.1234 --mount type=bind,source=$(pwd)/data,target=/data --user $(id -u):$(id -g) mennotammens/harmonie-grib:latest
  ```

- From GitHub:

  ```
  git clone git@github.com:MennoTammens/harmonie-grib.git
  cd harmonie-grib
  docker build -t harmonie-grib:latest .
  docker run --rm --name grib-download -e KNMI_API_KEY=1234567890abcdef -e METEOGRAM_LAT=51.2345 -e METEOGRAM_LON=6.1234 --mount type=bind,source=$(pwd)/data,target=/data --user $(id -u):$(id -g) harmonie-grib:latest
  ```
