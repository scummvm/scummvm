import io
import setuptools
from distutils.command.sdist import sdist as _sdist
import subprocess
import time

VERSION='1.15.0'
RELEASE='1'

class sdist(_sdist):
    """Custom sdist command, to prep pycdlib.spec file for inclusion."""

    def run(self):
        global VERSION
        global RELEASE

        # If development release, include date+githash in %{release}
        if RELEASE.startswith('0'):
            # Create a development release string for later use
            git_head = subprocess.Popen("git log -1 --pretty=format:%h",
                                        shell=True,
                                        stdout=subprocess.PIPE).communicate()[0].strip()
            date = time.strftime("%Y%m%d%H%M%S", time.gmtime())
            git_release = "%sgit%s" % (date, git_head.decode('utf-8'))
            RELEASE += '.' + git_release

        # Expand macros in pycdlib.spec.in and create pycdlib.spec
        with open('python-pycdlib.spec.in', 'r') as spec_in:
            with open('python-pycdlib.spec', 'w') as spec_out:
                for line in spec_in:
                    if "@VERSION@" in line:
                        line = line.replace("@VERSION@", VERSION)
                    elif "@RELEASE@" in line:
                        line = line.replace("@RELEASE@", RELEASE)
                    spec_out.write(line)

        # Run parent constructor
        _sdist.run(self)

setuptools.setup(name='pycdlib',
                 version=VERSION,
                 description='Pure python ISO manipulation library',
                 long_description=io.open('README.md', encoding='UTF-8').read(),
                 url='http://github.com/clalancette/pycdlib',
                 author='Chris Lalancette',
                 author_email='clalancette@gmail.com',
                 license='LGPLv2',
                 classifiers=['Development Status :: 5 - Production/Stable',
                              'Intended Audience :: Developers',
                              'License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)',
                              'Natural Language :: English',
                              'Programming Language :: Python :: 3.7',
                 ],
                 keywords='iso9660 iso ecma119 rockridge joliet eltorito udf',
                 packages=['pycdlib'],
                 package_data={'': ['examples/*.py'], 'pycdlib': ['py.typed']},
                 cmdclass={'sdist': sdist},
                 data_files=[('share/man/man1', ['man/pycdlib-explorer.1', 'man/pycdlib-extract-files.1', 'man/pycdlib-genisoimage.1'])],
                 scripts=['tools/pycdlib-explorer', 'tools/pycdlib-extract-files', 'tools/pycdlib-genisoimage'],
)
