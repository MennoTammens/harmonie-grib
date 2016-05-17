#!/usr/bin/env python

import json

from convert import BOUNDS


def geojson_feature(feature_type, coordinates, properties=None, **kwargs):
    properties = properties or {}
    properties.update(kwargs)
    return {
        'type': 'Feature',
        'properties': properties,
        'geometry': {
            'type': feature_type,
            'coordinates': coordinates
        }
    }


def geojson_bounds(bounds, properties=None, **kwargs):
    '''Convert a tuple of (sw, ne)-corners to a geojson polygon'''

    sw_lng, sw_lat = bounds[0]
    ne_lng, ne_lat = bounds[1]
    return geojson_feature('Polygon', [
        [
            bounds[0],
            (sw_lng, ne_lat),
            bounds[1],
            (ne_lng, sw_lat)
        ]
    ], properties, **kwargs)

if __name__ == '__main__':
    all_bounds = []
    for key, bounds in BOUNDS.items():
        bounds = geojson_bounds(bounds, name=key)
        all_bounds.append(bounds)

        with open('{}.geojson'.format(key), 'w') as out:
            json.dump(bounds, out, indent=2)

    with open('harm36_bounds.geojson', 'w') as out:
        json.dump({
            'type': 'FeatureCollection',
            'features': all_bounds
        }, out, indent=2)


    print('Wrote bounds to .geojson files')
