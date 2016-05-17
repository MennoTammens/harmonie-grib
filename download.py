#!/usr/bin/env python
from __future__ import division, print_function

from datetime import datetime
from ftplib import FTP

FTP_HOST = 'data.knmi.nl'
BASE_PATH = '/download/harmonie_p1/0.2/noversion/0000/00/00/'
DATE_FMT = '%b %d %H:%M'

ftp = FTP(FTP_HOST, 'anonymous')
ftp.cwd(BASE_PATH)


def file_list():
    data = []
    ftp.dir(data.append)

    for item in data:
        item = item.split()
        mtime = datetime.strptime(' '.join(item[5:8]), DATE_FMT)

        yield {
            'name': item[-1],
            'size': '{}MB'.format(int(int(item[4]) / 1024 / 1024)),
            'mtime': mtime.replace(year=2016)  # TODO: fix year assumption
        }


def get_file(name):
    ftp.retrbinary('RETR {}'.format(name), open(name, 'wb').write)


print('Files available on server: ')
for item in file_list():
    print(' {}: {}'.format(item['name'], item['mtime'].strftime('%Y-%m-%d %H:%M:%S')))
print()

recent = max(file_list(), key=lambda x: x['mtime'])
print('Downloading most recent file:', recent['name'], 'size:', recent['size'])
get_file(recent['name'])
print('DONE')
