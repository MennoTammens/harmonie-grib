# Conversion of GRIB-files for HARMONIE-model

## Introduction
From [harmonie-grib](https://github.com/MennoTammens/harmonie-grib) (in Dutch):
- Het KNMI stelt in het [KNMI Data Platform](https://dataplatform.knmi.nl/catalog/index.html) GRIB-files van het Harmonie-model vrij beschikbaar als [Open Data](http://creativecommons.org/publicdomain/mark/1.0/deed.nl). Het HARMONIE-AROME Cy40-model levert data tot 48 uur vooruit met een hoge resolutie (2,5×2,5km) en in tijdstappen van 1 uur.
De GRIB-files van het KNMI zijn nog niet direct in GRIB-viewers als [XyGrib](https://opengribs.org/en/xygrib) te openen omdat het formaat een beetje afwijkt van wat de GRIB-viewers verwachten. Ik heb daarom de GRIB-files geconverteerd in een formaat dat wel te gebruiken is in XyGrib en diverse andere GRIB-viewers
- Voor meer informatie en discussie, zie dit topic op Zeilersforum: https://zeilersforum.nl/index.php/forum-125/meteo/575942-hoge-resolutie-grib-files

### Modification for wind data

**NOTE OCTOBER 2023**: As of Summer 2023, the HARMONIE data products are being updated 
(see [this newsletter](https://www.knmidata.nl/actueel/nieuwsbrieven/open-data-nieuwsbrief/2023/knmi-open-data-nieuwsbrief-juli) by the KNMI in Dutch).
The data products are expected to be updated end of 2023. New links have been added below but contain less information on the content of the products 
(e.g. on available levels).  

For the crane radar, we only need the U- and V-components for the wind forecasts in the data products offered by the KNMI for the HARMONIE-model:
- [New link DP1](https://dataplatform.knmi.nl/dataset/harmonie-arome-cy40-p1-0-2) 
  - [Old link DP1](https://www.knmidata.nl/data-services/knmi-producten-overzicht/atmosfeer-modeldata/data-product-1)
  - DP1 contains wind speed at specific height, with a maximum of 300 meters above ground.
  - Used a regular LatLon-representation.
- [New link DP2](https://dataplatform.knmi.nl/dataset/harmonie-arome-cy40-p3-0-2) 
  - [Old link DP2](https://www.knmidata.nl/data-services/knmi-producten-overzicht/atmosfeer-modeldata/data-product-2)
  - DP2 contains wind speed at specific pressure levels, with a maximum of 925 hPa, which are best suited for our use case.
  - Uses a rotated LatLon-representation.
  
To do, the app in this modified repository can be run with instructions provided below under *Usage*.
We removed the extraction of other elements in the HARMONIE-model for simplicity, including the meteogram.

**_NOTE:_** 
- The resulting GRIB-files were successfully tested and visualized on MacOS (M1-chip) using the app Panoply, for both DP1 and DP2. 
- However, the result for DP2 was not readable using XyGrib, nor in the existing shiny app for the crane radar. 
Presumably, this is due to the Rotated Lat/Lon-representation for DP2.
- For now, we will proceed with the wind-data from DP1 at 300 meters height.

## Usage

1. Get yourself an KNMI API key
2. To build from GitHub:
    ```
    git clone git@github.com:ralphdewit/harmonie-grib.git
    cd harmonie-grib
    docker build -t harmonie-grib:latest .
    ```
3. To run:
   1. to extract wind forecasts for the next 14 hours at 300 meters height from DP1 (data_product=1):
      ```
      docker run --rm --name grib-download -e KNMI_API_KEY=<your_key> -e TYPE_OF_LEVEL=heightAboveGround -e VALUE_OF_LEVEL=300 -e DATA_PRODUCT=1 -e HOUR_MAX=14 --mount type=bind,source=$(pwd)/data,target=/data --user $(id -u):$(id -g) harmonie-grib:latest
      ```
   2. to extract wind forecasts for the next 14 hours at 925 hPa from DP2 (data_product=3):
      ```
      docker run --rm --name grib-download -e KNMI_API_KEY=<your_key> -e TYPE_OF_LEVEL=isobaricInhPa -e VALUE_OF_LEVEL=925 -e DATA_PRODUCT=3 -e HOUR_MAX=14 --mount type=bind,source=$(pwd)/data,target=/data --user $(id -u):$(id -g) harmonie-grib:latest
      ```
    
N.B. The original repository also describes how to run via DockerHub ([harmonie-grib](https://github.com/MennoTammens/harmonie-grib), in Dutch).

## Running a scheduled job

The downloading and conversion of data can also be done through a scheduled job using *cron*-services and *Docker*. 
There are two ways to do so:
1. Using a Dockerfile
2. Using docker-compose

For both options, you need to specify some environment variables in a file `/jobs/.env` with the following structure.

For DP1, use:
```
KNMI_API_KEY=<your_key>
DATA_PRODUCT=1
DELETE_OLD_FILES=1
TYPE_OF_LEVEL=heightAboveGround
VALUE_OF_LEVEL=300
HOUR_MAX=14
```

For DP2, use:
```
KNMI_API_KEY=<your_key>
DATA_PRODUCT=3
DELETE_OLD_FILES=1
TYPE_OF_LEVEL=isobaricInhPa
VALUE_OF_LEVEL=925
HOUR_MAX=14
```

Of course, you can change these values whenever necessary. 

### Cron-service using a Dockerfile

To launch a cron service which checks for new data, and if available, performs the conversion, use the `cron.Dockerfile`.

```
docker build -f cron.Dockerfile -t harmonie-grib .
docker run -d --mount type=bind,source=$(pwd)/data,target=/data --env-file=jobs/.env --name harmonie_converter harmonie-grib
```

This should launch the cron job in a docker container, which is **scheduled to run every 30 minutes**. 
The output should become available in a local `data/`-folder (change this when needed).

To check all active cron jobs in your Docker:
```
docker exec -it harmonie_converter /bin/bash
crontab -l
```

To check the output log of the actual command being executed (as specified in `jobs/download_data.sh`):
```
cat /var/log/cron.log
```

**NOTE:** the log will be empty until the first execution of the command (i.e. at the beginning of the next hour).

