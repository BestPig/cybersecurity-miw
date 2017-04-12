DESTDIR = /usr/local/bin

all:
	python2 setup.py build

clean:
	rm -rf miwlib.egg-info/

install:
	pip2 install -r requirements.txt
	python2 setup.py install
	cp cli/json2pb $(DESTDIR)
	rm -rf build
	rm -rf dist
	rm -rf miwlib.egg-info/
