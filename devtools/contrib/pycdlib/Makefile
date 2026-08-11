clean:
	rm -rf htmlcov python-pycdlib.spec dist MANIFEST .coverage profile build *.lprof .mypy_cache
	find . -iname '*~' -exec rm -f {} \;
	find . -iname '*.pyc' -exec rm -f {} \;

deb:
	debuild -i -uc -us -b

docs:
	groff -mandoc -Thtml man/pycdlib-explorer.1 > docs/pycdlib-explorer.html
	groff -mandoc -Thtml man/pycdlib-extract-files.1 > docs/pycdlib-extract-files.html
	groff -mandoc -Thtml man/pycdlib-genisoimage.1 > docs/pycdlib-genisoimage.html
	python3 custom-pydoc.py > docs/pycdlib-api.html

flake8:
	-flake8-3 --ignore=E501,E266 --max-complexity 80 pycdlib tools/*

# kernprof-3 comes from the "line_profiler" package.  It allows performance
# profiling on a line-by-line basis, but needs to be told which functions to
# profile by using an "@profile" decorator on particular functions.  The easiest
# way to use this is to profile using the built-in cProfile module (like the
# "profile" target), then mark the hotspots with "@profile", and then run
# the "lineprof" target.
lineprof:
	kernprof-3 -v -l /usr/bin/py.test-3 --verbose tests

mypy:
	mypy --ignore-missing-imports -m pycdlib

profile:
	python3 -m cProfile -o profile /usr/bin/py.test-3 --verbose tests
	python3 -c "import pstats; p=pstats.Stats('profile');p.strip_dirs();p.sort_stats('time').print_stats(30)"

pylint:
	-pylint-3 --rcfile=pylint.conf pycdlib tools/*

rpm: sdist
	rpmbuild -ba python-pycdlib.spec --define "_sourcedir `pwd`/dist"

sdist:
	python3 setup.py sdist

slowtests:
	PYCDLIB_TRACK_WRITES=1 py.test-3 --basetemp=/var/tmp/pycdlib-tests --runslow --verbose tests

srpm: sdist
	rpmbuild -bs python-pycdlib.spec --define "_sourcedir `pwd`/dist"

test-coverage:
	PYCDLIB_TRACK_WRITES=1 coverage3 run --source pycdlib /usr/bin/py.test-3 --basetemp=/var/tmp/pycdlib-tests --runslow --verbose tests
	coverage3 html
	xdg-open htmlcov/index.html

tests:
	py.test-3 --verbose tests

.PHONY: clean deb docs flake8 lineprof mypy profile pylint rpm sdist slowtests srpm test-coverage tests
