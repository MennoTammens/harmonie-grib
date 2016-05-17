TMPDIR := work

all: download convert


convert:
	mkdir -p $(TMPDIR)/
	tar -xf harm36_v1_ned_surface_*.tgz -C $(TMPDIR)
	./convert.py

download:
	./download.py

ggrib:
	wget http://ftp.cpc.ncep.noaa.gov/wd51we/gribw/ggrib/ggrib.tar.gz -O $(TMPDIR)/ggrib.tar.gz
	tar -xf $(TMPDIR)/ggrib.tar.gz -C $(TMPDIR)

	cp $(TMPDIR)/Ggrib/ggrib .

bounds:
	./bounds.py

clean:
	rm -rf $(TMPDIR)/Ggrib
	rm -rf $(TMPDIR)/Gribw
	rm -rf $(TMPDIR)/ggrib.tar.gz
	rm -rf $(TMPDIR)/harm36_v1_ned_surface_*
	rm -rf temp.grb
