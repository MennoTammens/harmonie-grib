#!/bin/bash

#########
# This job downloads data by executing /app/cron.py every hour (minute 0, e.g. at 13:00).
# For background on cron jobs, see e.g. https://crontab.guru/.
#
# To check that the cron job is running:
# "docker exec -it harmonie_converter /bin/bash" (to access your running Docker container)
# "crontab -l" (to list all active cron jobs)
#
# Each hour, the output of /app/cron.py is logged and can be checked used e.g. "cat /var/log/cron.log".
# NOTE: the log will be empty until the first execution (i.e. at the beginning of the next hour).
#########

# execute grib downloader and converter and write stdout to file
0 * * * * export $(xargs < /jobs/.env); python3 /app/cron.py >>/var/log/cron.log 2>&1
