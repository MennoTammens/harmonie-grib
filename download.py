#!/usr/bin/env python
from __future__ import division, print_function

import os
from datetime import datetime
from ftplib import FTP

FTP_HOST = 'data.knmi.nl'
BASE_PATH = '/download/harmonie_p1/0.2/noversion/0000/00/00/'
DATE_FMT = '%b %d %H:%M'

TMPDIR = 'tmp/'
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
            'mtime': mtime.replace(year=datetime.now().year)
        }


def get_file(name):
    ftp.retrbinary('RETR {}'.format(name), open(TMPDIR + name, 'wb').write)


def mtime(filename):
    return datetime.fromtimestamp(os.path.getmtime(filename))


print('Files available at {}: '.format(FTP_HOST))
for item in file_list():
    print(' {}: {}'.format(item['name'], item['mtime'].strftime('%Y-%m-%d %H:%M:%S')))
print()

recent = max(file_list(), key=lambda x: x['mtime'])

if os.path.isfile(TMPDIR + recent['name']) and recent['mtime'] < mtime(TMPDIR + recent['name']):
    print('Skipping download, {} already up to date'.format(recent['name']))
else:
    print('Downloading: {name}, size: {size}'.format(**recent))

    get_file(recent['name'])
print('DONE')
