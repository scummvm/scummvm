def parse_args(text):
	#print "parsing: [%s]" %text
	escape = False
	string = False
	result = []
	token = str()
	value = 0;
	for c in text:
		#print "[%s]%s: %s: %s" %(token, c, escape, string)
		if c == '\\':
			escape = True
			continue

		if escape:
			if not string:
				raise SyntaxError("escape found in no string: %s" %text);
	
			#print "escaping[%s]" %c
			escape = False
			token += c
			continue

		if string:
			if c == '\'' or c == '"':
				string = False

			token += c
			continue
		
		if c == '\'' or c == '"':
			string = True
			token += c
			continue

		if c == ',':
			result.append(token.strip())
			token = str()
			continue

		if c == ';': #comment, bailing out
			break
		
		token += c
	#token = token.strip()
	if len(token):
		result.append(token)
	#print result
	return result

def compile(width, data):
	print data
	return data
