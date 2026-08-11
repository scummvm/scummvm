# As of Python 3.11, the ability to write out API documentation with the
# pydoc module (the equivalent of 'pydoc -w <module>') seems completely
# broken.  The code below is a lightly-modified version of the pydoc
# module that makes this actually work.  It then goes on to remove the "Modules"
# section and the absolute links.

import __future__
import builtins
import collections
import importlib
import inspect
import os
import platform
import re
import reprlib
import sys
import sysconfig
import urllib.parse

def _split_list(s, predicate):
    """Split sequence s via predicate, and return pair ([true], [false]).

    The return value is a 2-tuple of lists,
        ([x for x in s if predicate(x)],
         [x for x in s if not predicate(x)])
    """

    yes = []
    no = []
    for x in s:
        if predicate(x):
            yes.append(x)
        else:
            no.append(x)
    return yes, no

_future_feature_names = set(__future__.all_feature_names)

def _is_bound_method(fn):
    """
    Returns True if fn is a bound method, regardless of whether
    fn was implemented in Python or in C.
    """
    if inspect.ismethod(fn):
        return True
    if inspect.isbuiltin(fn):
        self = getattr(fn, '__self__', None)
        return not (inspect.ismodule(self) or (self is None))
    return False

def classify_class_attrs(object):
    """Wrap inspect.classify_class_attrs, with fixup for data descriptors and bound methods."""
    results = []
    for (name, kind, cls, value) in inspect.classify_class_attrs(object):
        if inspect.isdatadescriptor(value):
            kind = 'data descriptor'
            if isinstance(value, property) and value.fset is None:
                kind = 'readonly property'
        elif kind == 'method' and _is_bound_method(value):
            kind = 'static method'
        results.append((name, kind, cls, value))
    return results

def sort_attributes(attrs, object):
    'Sort the attrs list in-place by _fields and then alphabetically by name'
    # This allows data descriptors to be ordered according
    # to a _fields attribute if present.
    fields = getattr(object, '_fields', [])
    try:
        field_order = {name : i-len(fields) for (i, name) in enumerate(fields)}
    except TypeError:
        field_order = {}
    keyfunc = lambda attr: (field_order.get(attr[0], 0), attr[0])
    attrs.sort(key=keyfunc)

def _findclass(func):
    cls = sys.modules.get(func.__module__)
    if cls is None:
        return None
    for name in func.__qualname__.split('.')[:-1]:
        cls = getattr(cls, name)
    if not inspect.isclass(cls):
        return None
    return cls

def _finddoc(obj):
    if inspect.ismethod(obj):
        name = obj.__func__.__name__
        self = obj.__self__
        if (inspect.isclass(self) and
            getattr(getattr(self, name, None), '__func__') is obj.__func__):
            # classmethod
            cls = self
        else:
            cls = self.__class__
    elif inspect.isfunction(obj):
        name = obj.__name__
        cls = _findclass(obj)
        if cls is None or getattr(cls, name) is not obj:
            return None
    elif inspect.isbuiltin(obj):
        name = obj.__name__
        self = obj.__self__
        if (inspect.isclass(self) and
            self.__qualname__ + '.' + name == obj.__qualname__):
            # classmethod
            cls = self
        else:
            cls = self.__class__
    # Should be tested before isdatadescriptor().
    elif isinstance(obj, property):
        func = obj.fget
        name = func.__name__
        cls = _findclass(func)
        if cls is None or getattr(cls, name) is not obj:
            return None
    elif inspect.ismethoddescriptor(obj) or inspect.isdatadescriptor(obj):
        name = obj.__name__
        cls = obj.__objclass__
        if getattr(cls, name) is not obj:
            return None
        if inspect.ismemberdescriptor(obj):
            slots = getattr(cls, '__slots__', None)
            if isinstance(slots, dict) and name in slots:
                return slots[name]
    else:
        return None
    for base in cls.__mro__:
        try:
            doc = _getowndoc(getattr(base, name))
        except AttributeError:
            continue
        if doc is not None:
            return doc
    return None

def _getowndoc(obj):
    """Get the documentation string for an object if it is not
    inherited from its class."""
    try:
        doc = object.__getattribute__(obj, '__doc__')
        if doc is None:
            return None
        if obj is not type:
            typedoc = type(obj).__doc__
            if isinstance(typedoc, str) and typedoc == doc:
                return None
        return doc
    except AttributeError:
        return None

def _getdoc(object):
    """Get the documentation string for an object.

    All tabs are expanded to spaces.  To clean up docstrings that are
    indented to line up with blocks of code, any whitespace than can be
    uniformly removed from the second line onwards is removed."""
    doc = _getowndoc(object)
    if doc is None:
        try:
            doc = _finddoc(object)
        except (AttributeError, TypeError):
            return None
    if not isinstance(doc, str):
        return None
    return inspect.cleandoc(doc)

def getdoc(object):
    """Get the doc string or comments for an object."""
    result = _getdoc(object) or inspect.getcomments(object)
    return result and re.sub('^ *\n', '', result.rstrip()) or ''

def isdata(object):
    """Check if an object is of a type that probably means it's data."""
    return not (inspect.ismodule(object) or inspect.isclass(object) or
                inspect.isroutine(object) or inspect.isframe(object) or
                inspect.istraceback(object) or inspect.iscode(object))

def visiblename(name, all=None, obj=None):
    """Decide whether to show documentation on a variable."""
    # Certain special names are redundant or internal.
    # XXX Remove __initializing__?
    if name in {'__author__', '__builtins__', '__cached__', '__credits__',
                '__date__', '__doc__', '__file__', '__spec__',
                '__loader__', '__module__', '__name__', '__package__',
                '__path__', '__qualname__', '__slots__', '__version__'}:
        return 0
    # Private names are hidden, but special names are displayed.
    if name.startswith('__') and name.endswith('__'): return 1
    # Namedtuples have public fields and methods with a single leading underscore
    if name.startswith('_') and hasattr(obj, '_fields'):
        return True
    # Ignore __future__ imports.
    if obj is not __future__ and name in _future_feature_names:
        if isinstance(getattr(obj, name, None), __future__._Feature):
            return False
    if all is not None:
        # only document that which the programmer exported in __all__
        return name in all
    else:
        return not name.startswith('_')

def describe(thing):
    """Produce a short description of the given thing."""
    if inspect.ismodule(thing):
        if thing.__name__ in sys.builtin_module_names:
            return 'built-in module ' + thing.__name__
        if hasattr(thing, '__path__'):
            return 'package ' + thing.__name__
        else:
            return 'module ' + thing.__name__
    if inspect.isbuiltin(thing):
        return 'built-in function ' + thing.__name__
    if inspect.isgetsetdescriptor(thing):
        return 'getset descriptor %s.%s.%s' % (
            thing.__objclass__.__module__, thing.__objclass__.__name__,
            thing.__name__)
    if inspect.ismemberdescriptor(thing):
        return 'member descriptor %s.%s.%s' % (
            thing.__objclass__.__module__, thing.__objclass__.__name__,
            thing.__name__)
    if inspect.isclass(thing):
        return 'class ' + thing.__name__
    if inspect.isfunction(thing):
        return 'function ' + thing.__name__
    if inspect.ismethod(thing):
        return 'method ' + thing.__name__
    return type(thing).__name__

def safeimport(path, forceload=0, cache={}):
    """Import a module; handle errors; return None if the module isn't found.

    If the module *is* found but an exception occurs, it's wrapped in an
    ErrorDuringImport exception and reraised.  Unlike __import__, if a
    package path is specified, the module at the end of the path is returned,
    not the package at the beginning.  If the optional 'forceload' argument
    is 1, we reload the module from disk (unless it's a dynamic extension)."""
    try:
        # If forceload is 1 and the module has been previously loaded from
        # disk, we always have to reload the module.  Checking the file's
        # mtime isn't good enough (e.g. the module could contain a class
        # that inherits from another module that has changed).
        if forceload and path in sys.modules:
            if path not in sys.builtin_module_names:
                # Remove the module from sys.modules and re-import to try
                # and avoid problems with partially loaded modules.
                # Also remove any submodules because they won't appear
                # in the newly loaded module's namespace if they're already
                # in sys.modules.
                subs = [m for m in sys.modules if m.startswith(path + '.')]
                for key in [path] + subs:
                    # Prevent garbage collection.
                    cache[key] = sys.modules[key]
                    del sys.modules[key]
        module = importlib.import_module(path)
    except BaseException as err:
        # Did the error occur before or after the module was found?
        if path in sys.modules:
            # An error occurred while executing the imported module.
            raise ErrorDuringImport(sys.modules[path].__file__, err)
        elif type(err) is SyntaxError:
            # A SyntaxError occurred before we could execute the module.
            raise ErrorDuringImport(err.filename, err)
        elif isinstance(err, ImportError) and err.name == path:
            # No such module in the path.
            return None
        else:
            # Some other error occurred during the importing process.
            raise ErrorDuringImport(path, err)
    return module

def locate(path, forceload=0):
    """Locate an object by name or dotted path, importing as necessary."""
    parts = [part for part in path.split('.') if part]
    module, n = None, 0
    while n < len(parts):
        nextmodule = safeimport('.'.join(parts[:n+1]), forceload)
        if nextmodule: module, n = nextmodule, n + 1
        else: break
    if module:
        object = module
    else:
        object = builtins
    for part in parts[n:]:
        try:
            object = getattr(object, part)
        except AttributeError:
            return None
    return object

class Doc:

    PYTHONDOCS = os.environ.get("PYTHONDOCS",
                                "https://docs.python.org/%d.%d/library"
                                % sys.version_info[:2])

    def document(self, object, name=None, *args):
        """Generate documentation for an object."""
        args = (object, name) + args
        # 'try' clause is to attempt to handle the possibility that inspect
        # identifies something in a way that pydoc itself has issues handling;
        # think 'super' and how it is a descriptor (which raises the exception
        # by lacking a __name__ attribute) and an instance.
        try:
            if inspect.ismodule(object): return self.docmodule(*args)
            if inspect.isclass(object): return self.docclass(*args)
            if inspect.isroutine(object): return self.docroutine(*args)
        except AttributeError:
            pass
        if inspect.isdatadescriptor(object): return self.docdata(*args)
        return self.docother(*args)

    def fail(self, object, name=None, *args):
        """Raise an exception for unimplemented types."""
        message = "don't know how to document object%s of type %s" % (
            name and ' ' + repr(name), type(object).__name__)
        raise TypeError(message)

    docmodule = docclass = docroutine = docother = docproperty = docdata = fail

    def getdocloc(self, object, basedir=sysconfig.get_path('stdlib')):
        """Return the location of module docs or None"""

        try:
            file = inspect.getabsfile(object)
        except TypeError:
            file = '(built-in)'

        docloc = os.environ.get("PYTHONDOCS", self.PYTHONDOCS)

        basedir = os.path.normcase(basedir)
        if (isinstance(object, type(os)) and
            (object.__name__ in ('errno', 'exceptions', 'gc',
                                 'marshal', 'posix', 'signal', 'sys',
                                 '_thread', 'zipimport') or
             (file.startswith(basedir) and
              not file.startswith(os.path.join(basedir, 'site-packages')))) and
            object.__name__ not in ('xml.etree', 'test.test_pydoc.pydoc_mod')):
            if docloc.startswith(("http://", "https://")):
                docloc = "{}/{}.html".format(docloc.rstrip("/"), object.__name__.lower())
            else:
                docloc = os.path.join(docloc, object.__name__.lower() + ".html")
        else:
            docloc = None
        return docloc

def classname(object, modname):
    """Get a class name and qualify it with a module name if necessary."""
    name = object.__name__
    if object.__module__ != modname:
        name = object.__module__ + '.' + name
    return name

def replace(text, *pairs):
    """Do a series of global replacements on a string."""
    while pairs:
        text = pairs[1].join(text.split(pairs[0]))
        pairs = pairs[2:]
    return text

class HTMLRepr(reprlib.Repr):
    """Class for safely making an HTML representation of a Python object."""
    def __init__(self):
        reprlib.Repr.__init__(self)
        self.maxlist = self.maxtuple = 20
        self.maxdict = 10
        self.maxstring = self.maxother = 100

    def escape(self, text):
        return replace(text, '&', '&amp;', '<', '&lt;', '>', '&gt;')

    def repr(self, object):
        return reprlib.Repr.repr(self, object)

    def repr1(self, x, level):
        if hasattr(type(x), '__name__'):
            methodname = 'repr_' + '_'.join(type(x).__name__.split())
            if hasattr(self, methodname):
                return getattr(self, methodname)(x, level)
        return self.escape(cram(stripid(repr(x)), self.maxother))

    def repr_string(self, x, level):
        test = cram(x, self.maxstring)
        testrepr = repr(test)
        if '\\' in test and '\\' not in replace(testrepr, r'\\', ''):
            # Backslashes are only literal in the string and are never
            # needed to make any special characters, so show a raw string.
            return 'r' + testrepr[0] + self.escape(test) + testrepr[0]
        return re.sub(r'((\\[\\abfnrtv\'"]|\\[0-9]..|\\x..|\\u....)+)',
                      r'<span class="repr">\1</span>',
                      self.escape(testrepr))

    repr_str = repr_string

    def repr_instance(self, x, level):
        try:
            return self.escape(cram(stripid(repr(x)), self.maxstring))
        except:
            return self.escape('<%s instance>' % x.__class__.__name__)

    repr_unicode = repr_string

class HTMLDoc(Doc):
    """Formatter class for HTML documentation."""

    # ------------------------------------------- HTML formatting utilities

    _repr_instance = HTMLRepr()
    repr = _repr_instance.repr
    escape = _repr_instance.escape

    def page(self, title, contents):
        """Format an HTML page."""
        return '''\
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Python: %s</title>
</head><body>
%s
</body></html>''' % (title, contents)

    def heading(self, title, extras=''):
        """Format a page heading."""
        return '''
<table class="heading">
<tr class="heading-text decor">
<td class="title">&nbsp;<br>%s</td>
<td class="extra">%s</td></tr></table>
    ''' % (title, extras or '&nbsp;')

    def section(self, title, cls, contents, width=6,
                prelude='', marginalia=None, gap='&nbsp;'):
        """Format a section with a heading."""
        if marginalia is None:
            marginalia = '<span class="code">' + '&nbsp;' * width + '</span>'
        result = '''<p>
<table class="section">
<tr class="decor %s-decor heading-text">
<td class="section-title" colspan=3>&nbsp;<br>%s</td></tr>
    ''' % (cls, title)
        if prelude:
            result = result + '''
<tr><td class="decor %s-decor" rowspan=2>%s</td>
<td class="decor %s-decor" colspan=2>%s</td></tr>
<tr><td>%s</td>''' % (cls, marginalia, cls, prelude, gap)
        else:
            result = result + '''
<tr><td class="decor %s-decor">%s</td><td>%s</td>''' % (cls, marginalia, gap)

        return result + '\n<td class="singlecolumn">%s</td></tr></table>' % contents

    def bigsection(self, title, *args):
        """Format a section with a big heading."""
        title = '<strong class="bigsection">%s</strong>' % title
        return self.section(title, *args)

    def preformat(self, text):
        """Format literal preformatted text."""
        text = self.escape(text.expandtabs())
        return replace(text, '\n\n', '\n \n', '\n\n', '\n \n',
                             ' ', '&nbsp;', '\n', '<br>\n')

    def multicolumn(self, list, format):
        """Format a list of items into a multi-column list."""
        result = ''
        rows = (len(list) + 3) // 4
        for col in range(4):
            result = result + '<td class="multicolumn">'
            for i in range(rows*col, rows*col+rows):
                if i < len(list):
                    result = result + format(list[i]) + '<br>\n'
            result = result + '</td>'
        return '<table><tr>%s</tr></table>' % result

    def grey(self, text): return '<span class="grey">%s</span>' % text

    def namelink(self, name, *dicts):
        """Make a link for an identifier, given name-to-URL mappings."""
        for dict in dicts:
            if name in dict:
                return '<a href="%s">%s</a>' % (dict[name], name)
        return name

    def classlink(self, object, modname):
        """Make a link for a class."""
        name, module = object.__name__, sys.modules.get(object.__module__)
        if hasattr(module, name) and getattr(module, name) is object:
            return '<a href="%s.html#%s">%s</a>' % (
                module.__name__, name, classname(object, modname))
        return classname(object, modname)

    def parentlink(self, object, modname):
        """Make a link for the enclosing class or module."""
        link = None
        name, module = object.__name__, sys.modules.get(object.__module__)
        if hasattr(module, name) and getattr(module, name) is object:
            if '.' in object.__qualname__:
                name = object.__qualname__.rpartition('.')[0]
                if object.__module__ != modname:
                    link = '%s.html#%s' % (module.__name__, name)
                else:
                    link = '#%s' % name
            else:
                if object.__module__ != modname:
                    link = '%s.html' % module.__name__
        if link:
            return '<a href="%s">%s</a>' % (link, parentname(object, modname))
        else:
            return parentname(object, modname)

    def modulelink(self, object):
        """Make a link for a module."""
        return '<a href="%s.html">%s</a>' % (object.__name__, object.__name__)

    def modpkglink(self, modpkginfo):
        """Make a link for a module or package to display in an index."""
        name, path, ispackage, shadowed = modpkginfo
        if shadowed:
            return self.grey(name)
        if path:
            url = '%s.%s.html' % (path, name)
        else:
            url = '%s.html' % name
        if ispackage:
            text = '<strong>%s</strong>&nbsp;(package)' % name
        else:
            text = name
        return '<a href="%s">%s</a>' % (url, text)

    def filelink(self, url, path):
        """Make a link to source file."""
        return '<a href="file:%s">%s</a>' % (url, path)

    def markup(self, text, escape=None, funcs={}, classes={}, methods={}):
        """Mark up some plain text, given a context of symbols to look for.
        Each context dictionary maps object names to anchor names."""
        escape = escape or self.escape
        results = []
        here = 0
        pattern = re.compile(r'\b((http|https|ftp)://\S+[\w/]|'
                                r'RFC[- ]?(\d+)|'
                                r'PEP[- ]?(\d+)|'
                                r'(self\.)?(\w+))')
        while match := pattern.search(text, here):
            start, end = match.span()
            results.append(escape(text[here:start]))

            all, scheme, rfc, pep, selfdot, name = match.groups()
            if scheme:
                url = escape(all).replace('"', '&quot;')
                results.append('<a href="%s">%s</a>' % (url, url))
            elif rfc:
                url = 'https://www.rfc-editor.org/rfc/rfc%d.txt' % int(rfc)
                results.append('<a href="%s">%s</a>' % (url, escape(all)))
            elif pep:
                url = 'https://peps.python.org/pep-%04d/' % int(pep)
                results.append('<a href="%s">%s</a>' % (url, escape(all)))
            elif selfdot:
                # Create a link for methods like 'self.method(...)'
                # and use <strong> for attributes like 'self.attr'
                if text[end:end+1] == '(':
                    results.append('self.' + self.namelink(name, methods))
                else:
                    results.append('self.<strong>%s</strong>' % name)
            elif text[end:end+1] == '(':
                results.append(self.namelink(name, methods, funcs, classes))
            else:
                results.append(self.namelink(name, classes))
            here = end
        results.append(escape(text[here:]))
        return ''.join(results)

    # ---------------------------------------------- type-specific routines

    def formattree(self, tree, modname, parent=None):
        """Produce HTML for a class tree as given by inspect.getclasstree()."""
        result = ''
        for entry in tree:
            if isinstance(entry, tuple):
                c, bases = entry
                result = result + '<dt class="heading-text">'
                result = result + self.classlink(c, modname)
                if bases and bases != (parent,):
                    parents = []
                    for base in bases:
                        parents.append(self.classlink(base, modname))
                    result = result + '(' + ', '.join(parents) + ')'
                result = result + '\n</dt>'
            elif isinstance(entry, list):
                result = result + '<dd>\n%s</dd>\n' % self.formattree(
                    entry, modname, c)
        return '<dl>\n%s</dl>\n' % result

    def docmodule(self, object, name=None, mod=None, *ignored):
        """Produce HTML documentation for a module object."""
        name = object.__name__ # ignore the passed-in name
        try:
            all = object.__all__
        except AttributeError:
            all = None
        parts = name.split('.')
        links = []
        for i in range(len(parts)-1):
            links.append(
                '<a href="%s.html" class="white">%s</a>' %
                ('.'.join(parts[:i+1]), parts[i]))
        linkedname = '.'.join(links + parts[-1:])
        head = '<strong class="title">%s</strong>' % linkedname
        try:
            path = inspect.getabsfile(object)
            url = urllib.parse.quote(path)
            filelink = self.filelink(url, path)
        except TypeError:
            filelink = '(built-in)'
        info = []
        if hasattr(object, '__version__'):
            version = str(object.__version__)
            if version[:11] == '$' + 'Revision: ' and version[-1:] == '$':
                version = version[11:-1].strip()
            info.append('version %s' % self.escape(version))
        if hasattr(object, '__date__'):
            info.append(self.escape(str(object.__date__)))
        if info:
            head = head + ' (%s)' % ', '.join(info)
        docloc = self.getdocloc(object)
        if docloc is not None:
            docloc = '<br><a href="%(docloc)s">Module Reference</a>' % locals()
        else:
            docloc = ''

        result = self.heading(head, '')

        classes, cdict = [], {}
        for key, value in inspect.getmembers(object, inspect.isclass):
            # if __all__ exists, believe it.  Otherwise use old heuristic.
            if (all is not None or
                (inspect.getmodule(value) or object) is object):
                if visiblename(key, all, object):
                    classes.append((key, value))
                    cdict[key] = cdict[value] = '#' + key
        for key, value in classes:
            for base in value.__bases__:
                key, modname = base.__name__, base.__module__
                module = sys.modules.get(modname)
                if modname != name and module and hasattr(module, key):
                    if getattr(module, key) is base:
                        if not key in cdict:
                            cdict[key] = cdict[base] = modname + '.html#' + key
        funcs, fdict = [], {}
        for key, value in inspect.getmembers(object, inspect.isroutine):
            # if __all__ exists, believe it.  Otherwise use old heuristic.
            if (all is not None or
                inspect.isbuiltin(value) or inspect.getmodule(value) is object):
                if visiblename(key, all, object):
                    funcs.append((key, value))
                    fdict[key] = '#-' + key
                    if inspect.isfunction(value): fdict[value] = fdict[key]
        data = []
        for key, value in inspect.getmembers(object, isdata):
            if visiblename(key, all, object):
                data.append((key, value))

        doc = self.markup(getdoc(object), self.preformat, fdict, cdict)
        doc = doc and '<span class="code">%s</span>' % doc
        result = result + '<p>%s</p>\n' % doc

        if hasattr(object, '__path__'):
            modpkgs = []
            for importer, modname, ispkg in pkgutil.iter_modules(object.__path__):
                modpkgs.append((modname, name, ispkg, 0))
            modpkgs.sort()
            contents = self.multicolumn(modpkgs, self.modpkglink)
            result = result + self.bigsection(
                'Package Contents', 'pkg-content', contents)

        if classes:
            classlist = [value for (key, value) in classes]
            contents = [
                self.formattree(inspect.getclasstree(classlist, 1), name)]
            for key, value in classes:
                contents.append(self.document(value, key, name, fdict, cdict))
            result = result + self.bigsection(
                'Classes', 'index', ' '.join(contents))
        if funcs:
            contents = []
            for key, value in funcs:
                contents.append(self.document(value, key, name, fdict, cdict))
            result = result + self.bigsection(
                'Functions', 'functions', ' '.join(contents))
        if data:
            contents = []
            for key, value in data:
                contents.append(self.document(value, key))
            result = result + self.bigsection(
                'Data', 'data', '<br>\n'.join(contents))
        if hasattr(object, '__author__'):
            contents = self.markup(str(object.__author__), self.preformat)
            result = result + self.bigsection('Author', 'author', contents)
        if hasattr(object, '__credits__'):
            contents = self.markup(str(object.__credits__), self.preformat)
            result = result + self.bigsection('Credits', 'credits', contents)

        return result

    def docclass(self, object, name=None, mod=None, funcs={}, classes={},
                 *ignored):
        """Produce HTML documentation for a class object."""
        realname = object.__name__
        name = name or realname
        bases = object.__bases__

        contents = []
        push = contents.append

        # Cute little class to pump out a horizontal rule between sections.
        class HorizontalRule:
            def __init__(self):
                self.needone = 0
            def maybe(self):
                if self.needone:
                    push('<hr>\n')
                self.needone = 1
        hr = HorizontalRule()

        # List the mro, if non-trivial.
        mro = collections.deque(inspect.getmro(object))
        if len(mro) > 2:
            hr.maybe()
            push('<dl><dt>Method resolution order:</dt>\n')
            for base in mro:
                push('<dd>%s</dd>\n' % self.classlink(base,
                                                      object.__module__))
            push('</dl>\n')

        def spill(msg, attrs, predicate):
            ok, attrs = _split_list(attrs, predicate)
            if ok:
                hr.maybe()
                push(msg)
                for name, kind, homecls, value in ok:
                    try:
                        value = getattr(object, name)
                    except Exception:
                        # Some descriptors may meet a failure in their __get__.
                        # (bug #1785)
                        push(self.docdata(value, name, mod))
                    else:
                        push(self.document(value, name, mod,
                                        funcs, classes, mdict, object, homecls))
                    push('\n')
            return attrs

        def spilldescriptors(msg, attrs, predicate):
            ok, attrs = _split_list(attrs, predicate)
            if ok:
                hr.maybe()
                push(msg)
                for name, kind, homecls, value in ok:
                    push(self.docdata(value, name, mod))
            return attrs

        def spilldata(msg, attrs, predicate):
            ok, attrs = _split_list(attrs, predicate)
            if ok:
                hr.maybe()
                push(msg)
                for name, kind, homecls, value in ok:
                    base = self.docother(getattr(object, name), name, mod)
                    doc = getdoc(value)
                    if not doc:
                        push('<dl><dt>%s</dl>\n' % base)
                    else:
                        doc = self.markup(getdoc(value), self.preformat,
                                          funcs, classes, mdict)
                        doc = '<dd><span class="code">%s</span>' % doc
                        push('<dl><dt>%s%s</dl>\n' % (base, doc))
                    push('\n')
            return attrs

        attrs = [(name, kind, cls, value)
                 for name, kind, cls, value in classify_class_attrs(object)
                 if visiblename(name, obj=object)]

        mdict = {}
        for key, kind, homecls, value in attrs:
            mdict[key] = anchor = '#' + name + '-' + key
            try:
                value = getattr(object, name)
            except Exception:
                # Some descriptors may meet a failure in their __get__.
                # (bug #1785)
                pass
            try:
                # The value may not be hashable (e.g., a data attr with
                # a dict or list value).
                mdict[value] = anchor
            except TypeError:
                pass

        while attrs:
            if mro:
                thisclass = mro.popleft()
            else:
                thisclass = attrs[0][2]
            attrs, inherited = _split_list(attrs, lambda t: t[2] is thisclass)

            if object is not builtins.object and thisclass is builtins.object:
                attrs = inherited
                continue
            elif thisclass is object:
                tag = 'defined here'
            else:
                tag = 'inherited from %s' % self.classlink(thisclass,
                                                           object.__module__)
            tag += ':<br>\n'

            sort_attributes(attrs, object)

            # Pump out the attrs, segregated by kind.
            attrs = spill('Methods %s' % tag, attrs,
                          lambda t: t[1] == 'method')
            attrs = spill('Class methods %s' % tag, attrs,
                          lambda t: t[1] == 'class method')
            attrs = spill('Static methods %s' % tag, attrs,
                          lambda t: t[1] == 'static method')
            attrs = spilldescriptors("Readonly properties %s" % tag, attrs,
                                     lambda t: t[1] == 'readonly property')
            attrs = spilldescriptors('Data descriptors %s' % tag, attrs,
                                     lambda t: t[1] == 'data descriptor')
            attrs = spilldata('Data and other attributes %s' % tag, attrs,
                              lambda t: t[1] == 'data')
            assert attrs == []
            attrs = inherited

        contents = ''.join(contents)

        if name == realname:
            title = '<a name="%s">class <strong>%s</strong></a>' % (
                name, realname)
        else:
            title = '<strong>%s</strong> = <a name="%s">class %s</a>' % (
                name, name, realname)
        if bases:
            parents = []
            for base in bases:
                parents.append(self.classlink(base, object.__module__))
            title = title + '(%s)' % ', '.join(parents)

        decl = ''
        try:
            signature = inspect.signature(object)
        except (ValueError, TypeError):
            signature = None
        if signature:
            argspec = str(signature)
            if argspec and argspec != '()':
                decl = name + self.escape(argspec) + '\n\n'

        doc = getdoc(object)
        if decl:
            doc = decl + (doc or '')
        doc = self.markup(doc, self.preformat, funcs, classes, mdict)
        doc = doc and '<span class="code">%s<br>&nbsp;</span>' % doc

        return self.section(title, 'title', contents, 3, doc)

    def formatvalue(self, object):
        """Format an argument default value as text."""
        return self.grey('=' + self.repr(object))

    def docroutine(self, object, name=None, mod=None,
                   funcs={}, classes={}, methods={}, cl=None, homecls=None):
        """Produce HTML documentation for a function or method object."""
        realname = object.__name__
        name = name or realname
        if homecls is None:
            homecls = cl
        anchor = ('' if cl is None else cl.__name__) + '-' + name
        note = ''
        skipdocs = False
        imfunc = None
        if _is_bound_method(object):
            imself = object.__self__
            if imself is cl:
                imfunc = getattr(object, '__func__', None)
            elif inspect.isclass(imself):
                note = ' class method of %s' % self.classlink(imself, mod)
            else:
                note = ' method of %s instance' % self.classlink(
                    imself.__class__, mod)
        elif (inspect.ismethoddescriptor(object) or
              inspect.ismethodwrapper(object)):
            try:
                objclass = object.__objclass__
            except AttributeError:
                pass
            else:
                if cl is None:
                    note = ' unbound %s method' % self.classlink(objclass, mod)
                elif objclass is not homecls:
                    note = ' from ' + self.classlink(objclass, mod)
        else:
            imfunc = object
        if inspect.isfunction(imfunc) and homecls is not None and (
            imfunc.__module__ != homecls.__module__ or
            imfunc.__qualname__ != homecls.__qualname__ + '.' + realname):
            pname = self.parentlink(imfunc, mod)
            if pname:
                note = ' from %s' % pname

        if (inspect.iscoroutinefunction(object) or
                inspect.isasyncgenfunction(object)):
            asyncqualifier = 'async '
        else:
            asyncqualifier = ''

        if name == realname:
            title = '<a name="%s"><strong>%s</strong></a>' % (anchor, realname)
        else:
            if (cl is not None and
                inspect.getattr_static(cl, realname, []) is object):
                reallink = '<a href="#%s">%s</a>' % (
                    cl.__name__ + '-' + realname, realname)
                skipdocs = True
                if note.startswith(' from '):
                    note = ''
            else:
                reallink = realname
            title = '<a name="%s"><strong>%s</strong></a> = %s' % (
                anchor, name, reallink)
        argspec = None
        if inspect.isroutine(object):
            try:
                signature = inspect.signature(object)
            except (ValueError, TypeError):
                signature = None
            if signature:
                argspec = str(signature)
                if realname == '<lambda>':
                    title = '<strong>%s</strong> <em>lambda</em> ' % name
                    # XXX lambda's won't usually have func_annotations['return']
                    # since the syntax doesn't support but it is possible.
                    # So removing parentheses isn't truly safe.
                    if not object.__annotations__:
                        argspec = argspec[1:-1] # remove parentheses
        if not argspec:
            argspec = '(...)'

        decl = asyncqualifier + title + self.escape(argspec) + (note and
               self.grey('<span class="heading-text">%s</span>' % note))

        if skipdocs:
            return '<dl><dt>%s</dt></dl>\n' % decl
        else:
            doc = self.markup(
                getdoc(object), self.preformat, funcs, classes, methods)
            doc = doc and '<dd><span class="code">%s</span></dd>' % doc
            return '<dl><dt>%s</dt>%s</dl>\n' % (decl, doc)

    def docdata(self, object, name=None, mod=None, cl=None, *ignored):
        """Produce html documentation for a data descriptor."""
        results = []
        push = results.append

        if name:
            push('<dl><dt><strong>%s</strong></dt>\n' % name)
        doc = self.markup(getdoc(object), self.preformat)
        if doc:
            push('<dd><span class="code">%s</span></dd>\n' % doc)
        push('</dl>\n')

        return ''.join(results)

    docproperty = docdata

    def docother(self, object, name=None, mod=None, *ignored):
        """Produce HTML documentation for a data object."""
        lhs = name and '<strong>%s</strong> = ' % name or ''
        return lhs + self.repr(object)

    def index(self, dir, shadowed=None):
        """Generate an HTML index for a directory of modules."""
        modpkgs = []
        if shadowed is None: shadowed = {}
        for importer, name, ispkg in pkgutil.iter_modules([dir]):
            if any((0xD800 <= ord(ch) <= 0xDFFF) for ch in name):
                # ignore a module if its name contains a surrogate character
                continue
            modpkgs.append((name, '', ispkg, name in shadowed))
            shadowed[name] = 1

        modpkgs.sort()
        contents = self.multicolumn(modpkgs, self.modpkglink)
        return self.bigsection(dir, 'index', contents)

class _HTMLDoc(HTMLDoc):

    def page(self, title, contents):
        """Format an HTML page."""
        css_path = "pydoc_data/_pydoc.css"
        css_link = (
            '<link rel="stylesheet" type="text/css" href="%s">' %
            css_path)
        return '''\
<!DOCTYPE>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Pydoc: %s</title>
%s</head><body><div style="clear:both;padding-top:.5em;">%s</div>
</body></html>''' % (title, css_link, contents)

html = _HTMLDoc()

def html_getobj(url):
    obj = locate(url, forceload=1)
    if obj is None and url != 'None':
        raise ValueError('could not find object')
    title = describe(obj)
    content = html.document(obj, url)
    return title, content

title, content = html_getobj('pycdlib.pycdlib')
print(html.page(title, content))
