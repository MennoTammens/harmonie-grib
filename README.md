# Conversie GRIB-files HARMONIE-model

Het KNMI stelt in het KNMI Data Centrum GRIB-files van het Harmonie-model vrij beschikbaar als Open Data. Het Harmonie-model levert data tot 48 uur vooruit met een hoge resolutie (2.5×2.5km) en in tijdstappen van 1 uur.
De GRIB-files van het KNMI zijn nog niet direct in ZyGrib te openen omdat het formaat een beetje afwijkt van de GRIB-files uit het GFS-model. Dit script converteert daarom de GRIB-files in een formaat dat wel te gebruiken is in zyGrib.

Voor meer informatie en discussie, zie dit topic op Zeilersforum: http://zeilersforum.nl/index.php/forum-125/meteo/566725-knmi-basisgegevens-nu-beschikbaar

## Dependencies
- Python-module pygrib en de dependencies daarvan (numpy, grib-api, proj, zlib, libpng, jasper, openjpeg)
  in Debian/Ubuntu:

  ```
  sudo apt-get install python-grib
  ```

- [ggrib](http://www.cpc.ncep.noaa.gov/products/wesley/ggrib.html) voor het verkleinen van het gebied (kan ook zonder, dan moet je de regels met 'temp_nl.grb' en 'filename_nl' weghalen.)
