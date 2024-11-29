# Conversion of GRIB-files for KNMI HARMONIE weather model

## Introduction

This repository can be used to download GRIB-files for KNMI's HARMONIE-AROME weather models and extract specific layers from these files.
Here, we specifically extract wind data.

As of September 2024, this repository focuses on [KNMI HARMONIE-AROME Cy43 weather model](https://dataplatform.knmi.nl/dataset/harmonie-arome-cy43-p1-1-0).
Note that earlier, we used the (deprecated) [Cy40-model](https://dataplatform.knmi.nl/dataset/harmonie-arome-cy40-p1-0-2).
See [this newsletter](https://www.knmidata.nl/actueel/nieuwsbrieven/open-data-nieuwsbrief/2023/knmi-open-data-nieuwsbrief-juli) 
from 2023 by the KNMI in Dutch on the update to Cy43.

Some context on the Cy40-model from [harmonie-grib](https://github.com/MennoTammens/harmonie-grib) (*in Dutch*):
- Het KNMI stelt in het [KNMI Data Platform](https://dataplatform.knmi.nl/catalog/index.html) GRIB-files van het Harmonie-model vrij beschikbaar als [Open Data](http://creativecommons.org/publicdomain/mark/1.0/deed.nl). Het HARMONIE-AROME Cy40-model levert data tot 48 uur vooruit met een hoge resolutie (2,5×2,5km) en in tijdstappen van 1 uur.
De GRIB-files van het KNMI zijn nog niet direct in GRIB-viewers als [XyGrib](https://opengribs.org/en/xygrib) te openen omdat het formaat een beetje afwijkt van wat de GRIB-viewers verwachten. Ik heb daarom de GRIB-files geconverteerd in een formaat dat wel te gebruiken is in XyGrib en diverse andere GRIB-viewers
- Voor meer informatie en discussie, zie dit topic op Zeilersforum: https://zeilersforum.nl/index.php/forum-125/meteo/575942-hoge-resolutie-grib-files

### Modification for wind data

For the crane radar, we only need the U- and V-components for the wind forecasts in the data products offered by the KNMI for the HARMONIE-model:
- [HARMONIE-AROME Cy43 weather model](https://dataplatform.knmi.nl/dataset/harmonie-arome-cy43-p1-1-0)   
  - The data product contains wind speed at specific pressure levels, with a maximum of 925 hPa.
  - Uses a rotated LatLon-representation.
  
To do so, the app in this modified repository can be run with instructions provided below under *Usage*.
We removed the extraction of other elements in the HARMONIE-model for simplicity, including the meteogram.

**_NOTE on earlier extractions (2022/2023) for Cy40-model:_**
- In 2022/2023, we used the (now-deprecated) [Cy40-model](https://dataplatform.knmi.nl/dataset/harmonie-arome-cy40-p1-0-2).
- The resulting GRIB-files for the old Cy40-model were successfully tested and visualized on MacOS (M1-chip) using the app Panoply, for both DP1 and DP2. 
- However, the result for DP2 was not readable using XyGrib, nor in the existing shiny app for the crane radar. 
  - Presumably, this is due to the Rotated Lat/Lon-representation for DP2. We did use these files for DP2 in the end as the wind data at 925 hPa 
  was better suited for the crane radar application than wind data at 300 meters height.

## Usage

1. Get yourself an KNMI API key
2. To build from GitHub:
    ```
    git clone git@github.com:ralphdewit/harmonie-grib.git
    cd harmonie-grib
    docker build -t harmonie-grib:latest .
    ```
3. To run and extract wind forecasts for the next 14 hours at 925 hPa from Cy43:
    ```
    docker run --rm --name grib-download -e KNMI_API_KEY=<your_key> -e TYPE_OF_LEVEL=isobaricInhPa -e VALUE_OF_LEVEL=925 
    -e DATASET_ID=43 -e DATASET_PRODUCT=1 -e DATASET_VERSION=1.0 -e HOUR_MAX=14 
    --mount type=bind,source=$(pwd)/data,target=/data --user $(id -u):$(id -g) harmonie-grib:latest
    ```
    
N.B. The original repository also describes how to run via DockerHub ([harmonie-grib](https://github.com/MennoTammens/harmonie-grib), in Dutch).

## Running a scheduled job

The downloading and conversion of data can also be done through a scheduled job using *cron*-services and *Docker*. 
There are two ways to do so:
1. Using a Dockerfile
2. Using docker-compose

For both options, you need to specify some environment variables in a file `/jobs/.env` with the following structure:

```
KNMI_API_KEY=<your_key>
DATASET_ID=43
DATASET_PRODUCT=3
DATASET_VERSION=1.0
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

