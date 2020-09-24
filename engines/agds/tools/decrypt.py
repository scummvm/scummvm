key = bytes(map(lambda c: ord(c), 'Vyvojovy tym AGDS varuje: Hackerovani skodi obchodu!'))
key_n = len(key)

def decrypt(src):
	r = bytearray()
	for i in range(len(src)):
		r.append(src[i] ^ key[i % key_n] ^ 0xff)
	return r
