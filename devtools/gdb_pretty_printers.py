class StringPrinter:
    "Prints Common::String and Common::U32String"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val["_str"].string()

    def display_hint(self):
        return "string"


class ArrayPrinter:
    "Prints Common::Array"

    class _iterator:
        def __init__(self, start, item_count):
            self.current_item = start
            self.item_count = item_count
            self.current_item_count = 0
            self.item_size = 8 * start.dereference().type.sizeof

        def __iter__(self):
            return self

        def __next__(self):
            if self.current_item_count == self.item_count:
                raise StopIteration
            value = self.current_item.dereference()
            index = self.current_item_count
            self.current_item_count += 1
            self.current_item += 1
            return f"[{index}]", value

    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = self.val["_size"]
        capacity = self.val["_capacity"]
        return f"{self.val.type.name} of length {size}, capacity {capacity}"

    def children(self):
        return self._iterator(self.val["_storage"], self.val["_size"])

    def display_hint(self):
        return "array"

class RBTreeIterator:
    "Utility iterator for Common::RBTree"

    def __init__(self, leftmost, item_count):
        self.current_node = leftmost
        self.item_count = item_count
        self.current_item_count = 0

    def __iter__(self):
        return self

    def __next__(self):
        if self.current_item_count == self.item_count:
            raise StopIteration
        value = self.current_node.dereference()["value"]
        self.current_item_count += 1
        if self.current_node.dereference()["right"]:
            while self.current_node.dereference()["left"]:
                self.current_node = self.current_node.dereference()["left"]
        else:
            parent = self.current_node.dereference()["parent"]
            while parent and parent.dereference()["right"] == self.current_node.dereference()["right"]:
                self.current_node = parent
                parent = self.current_node.dereference()["parent"]
        return value

class StableMapPrinter:
    "Prints Common::StableMap"

    class _iterator:
        def __init__(self, rb_tree):
            self.iter = RBTreeIterator(rb_tree["_leftmost"], rb_tree["_size"])
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.count % 2 == 0:
                value = next(self.iter)
                self.current_pair = value
                item = value["first"]
            else:
                item = self.current_pair["second"]
            self.count += 1
            return f"{self.count}", item

    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = self.val["_items"]["_size"]
        return f"{self.val.type.name} of length {size}"

    def children(self):
        return self._iterator(self.val["_items"])

    def display_hint(self):
        return "map"


class ListPrinter:
    "Prints Common::List"

    class _iterator:
        def __init__(self, anchor, typename):
            self.anchor = anchor
            self.current_node = anchor["_next"].dereference()
            self.current_item_count = 0
            self.node_type = gdb.lookup_type(f"{typename}::Node")

        def __iter__(self):
            return self

        def __next__(self):
            if self.current_node.address == self.anchor.address:
                raise StopIteration
            value = self.current_node.cast(self.node_type)["_data"]
            index = self.current_item_count
            self.current_item_count += 1
            self.current_node = self.current_node["_next"].dereference()
            return f"[{index}]", value

    def __init__(self, val):
        self.val = val

    def to_string(self):
        if self.val["_anchor"]["_next"] == self.val["_anchor"].address:
            return f"empty {self.val.type.name}"
        return self.val.type.name

    def children(self):
        return self._iterator(self.val["_anchor"], self.val.type.name)

    def display_hint(self):
        return "array"

import gdb.printing

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter(
        "Common")
    pp.add_printer('String', '^Common::String$', StringPrinter)
    pp.add_printer('U32String', '^Common::U32String$', StringPrinter)
    pp.add_printer('Array', '^Common::Array<.*>$', ArrayPrinter)
    pp.add_printer('StableMap', '^Common::StableMap<.*>$', StableMapPrinter)
    pp.add_printer('List', '^Common::List<.*>$',  ListPrinter)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer()
)