FROM mennotammens/grib-tools:latest

COPY app /app
WORKDIR /app

CMD ["/app/cron.py"]
