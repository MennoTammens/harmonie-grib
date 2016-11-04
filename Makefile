TMPDIR := work

all: download convert


convert:
	mkdir -p $(TMPDIR)/
	tar -xf harm36_v1_ned_surface_*.tgz -C $(TMPDIR)
	./convert.py

download:
	./download.py

ggrib:
	$(MAKE) -C gribw -f gribwlib.make
	$(MAKE) -C gribw/ggrib -f ggrib.make
	cp gribw/ggrib/ggrib .

bounds:
	./bounds.py

clean:
	$(MAKE) -C gribw -f gribwlib.make clean
	rm ggrib
	rm -rf $(TMPDIR)/harm36_v1_ned_surface_*
	rm -rf temp.grb
