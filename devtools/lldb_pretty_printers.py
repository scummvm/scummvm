'''
This file contains LLDB pretty-printers for common ScummVM data types.
To enable it, add the following line to the .lldbinit file for this project:

    command script import devtools.lldb_pretty_printers

LLDB disallows loading .lldbinit files from source directories by default, so
you may need to add the following to your global ~/.lldbinit:

    settings set target.load-cwd-lldbinit true


'''
from dataclasses import dataclass
import lldb


class ArrayProvider:
    '''Formatter for Common::Array'''
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.data_type = self.valobj.GetChildMemberWithName("_storage").GetType().GetPointeeType()
        self.data_size = self.data_type.GetByteSize()
        self.update()

    def update(self):
        self.size = \
            self.valobj.GetChildMemberWithName("_size").GetValueAsUnsigned()
        return False

    def num_children(self, _max_children=None):
        return self.size

    def has_children(self):
        return self.size > 0

    def get_child_index(self, name):
        try:
            return int(name.lstrip("[").rstrip("]"))
        except ValueError:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        base = self.valobj.GetChildMemberWithName("_storage")
        offset = index * self.data_size
        return base.CreateChildAtOffset(
            f"[{index}]", offset, self.data_type
        )


class HashMapProvider:
    '''Formatter for Common::HashMap'''
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def update(self):
        self.children_with_values = []
        self.size = \
            self.valobj.GetChildMemberWithName("_size").GetValueAsUnsigned()
        self.mask = \
            self.valobj.GetChildMemberWithName("_mask").GetValueAsUnsigned()
        self.storage = self.valobj.GetChildMemberWithName("_storage")
        for i in range(self.mask + 1):
            child = self.storage.GetChildAtIndex(i, lldb.eNoDynamicValues, True)
            if child.GetValueAsUnsigned() != 0:
                self.children_with_values.append(child)
        return False

    def num_children(self, _max_children=None):
        return self.size

    def has_children(self):
        return self.size > 0

    def get_child_index(self, name):
        return next((c for c in self.children_with_values if c.GetName() == name), -1)

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None

        return self.children_with_values[index].Dereference()


class ListProvider:
    '''Formatter for Common::List'''
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.element_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.node_offset = self.valobj.GetType().FindDirectNestedType("NodeBase").GetByteSize()
        self.update()

    def update(self):
        self.anchor = self.valobj.GetChildMemberWithName("_anchor")

        self.size = 0
        cur = self.anchor
        while True:
            cur = cur.GetChildMemberWithName("_next").Dereference()
            if cur.AddressOf().GetValueAsUnsigned() == self.anchor.AddressOf().GetValueAsUnsigned():
                break
            self.size = self.size + 1
        return False

    def num_children(self, _max_children=None):
        return self.size

    def has_children(self):
        _prev = self.anchor.GetChildMemberWithName("_prev").GetValueAsUnsigned()
        _next = self.anchor.AddressOf().GetValueAsUnsigned()
        return _prev != _next

    def get_child_index(self, name):
        try:
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0 or index >= self.size:
            return None

        cur = self.anchor
        for _ in range(index):
            cur = cur.GetChildMemberWithName("_next").Dereference()

        return cur.CreateChildAtOffset(f'[{index}]', self.node_offset, self.element_type)


def array_summary(valobj, internal):
    size = valobj.GetNonSyntheticValue().GetChildMemberWithName('_size').GetValueAsUnsigned()
    if size == 0:
        return "(empty)"
    if size == 1:
        return "1 item"
    return f"{size} items"


def construct_matrix_summary(rows, cols):
    lines = []
    for r in range(rows):
        line = " ".join(f'${{var._values[{r*cols+c}]}}' for c in range(cols))
        lines.append(line)
    return '[' + "; ".join(lines) + ']'

@dataclass(frozen=True)
class Pattern:
    pattern: str

summaries = {
    'Common::String': r'${var._str%s}',

    'Common::Rect': r'(${var.left},${var.top}) -> (${var.right}, ${var.bottom})',
    'Math::Vector2d': r'(${var._values[0]}, ${var._values[1]})',
    'Math::Vector3d': r'(${var._values[0]}, ${var._values[1]}, ${var._values[2]})',
    'Math::Vector4d': r'(${var._values[0]}, ${var._values[1]}, ${var._values[2]}, ${var._values[3]})',
    'Math::Quaternion': r'(${var._values[0]}, ${var._values[1]}, ${var._values[2]}, ${var._values[3]})',
    'Math::Matrix4': construct_matrix_summary(4,4),
    'Math::Matrix3': construct_matrix_summary(3,3),

    'Graphics::Surface': r'(${var.w},${var.h}) (bpp=${var.format.bytesPerPixel:d})',
    Pattern('^Common::HashMap<.*>::Node'): r'\{key: ${var._key}, value: ${var._value}\}',
    Pattern('^Common::Array<.*>$'): array_summary,
    Pattern('^Common::List<.*>$'): r'${svar%#} items',
}

providers = {
    Pattern('^Common::Array<.*>$'): ArrayProvider,
    Pattern('^Common::HashMap<.*>$'): HashMapProvider,
    Pattern('^Common::List<.*>$'): ListProvider,
}


def __lldb_init_module(debugger, unused):
    #lldb.formatters.Logger._lldb_formatters_debug_level = 2
    print('Loading ScummVM formatters...')
    for klass, summary in summaries.items():
        if callable(summary):
            summary = f'-F {__name__}.{summary.__name__}'
        else:
            summary = f'--summary-string "{summary}"'

        if isinstance(klass, Pattern):
            debugger.HandleCommand(
                f'type summary add -x "{klass.pattern}" --category scummvm {summary}'
            )
        else:
            debugger.HandleCommand(
                f'type summary add "{klass}" --category scummvm {summary}'
            )

    for klass, provider in providers.items():
        debugger.HandleCommand(
            f'type synthetic add -x --category scummvm --python-class {__name__}.{provider.__name__} "{klass.pattern}"'
        )

    debugger.HandleCommand(
        r'type category enable scummvm'
    )
