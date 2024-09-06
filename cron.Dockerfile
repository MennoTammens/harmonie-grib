FROM mennotammens/grib-tools:latest

RUN apt-get update && apt-get -y install cron

COPY jobs /jobs
COPY app /app

# Set execute permissions for jobs
RUN find . -type f -iname "*.sh" -exec chmod +x {} \;

# create cron.log file
RUN touch /var/log/cron.log

WORKDIR /jobs

RUN crontab /jobs/download_data.sh

CMD printenv > /etc/environment && cron -f

