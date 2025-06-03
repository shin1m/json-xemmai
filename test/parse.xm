json = Module("json"
utilities = Module("utilities"
assert_equals = utilities.assert_equals
Source = utilities.Source
list_to_string = utilities.list_to_string

parse = @(text)
	list = [
	n = text.size(
	for i = 0; i < n; i = i + 1; list.push(text.code_at(i
	json.parse(Source(list).read
test = @(f)
	f(parse
	f(json.build

test(@(parse) assert_equals(parse(" null "), null
test(@(parse) assert_equals(parse(" false "), false
test(@(parse) assert_equals(parse(" true "), true
test(@(parse) assert_equals(parse(" 0 "), 0
test(@(parse) assert_equals(parse(" -1 "), -1
test(@(parse) assert_equals(parse(" 0.5 "), 0.5
test(@(parse) assert_equals(parse(" -1.0e+1 "), -10.0
test(@(parse) assert_equals(parse(" \"\" "), ""
test(@(parse) assert_equals(parse(" \"foo\\n\\u0009bar\" "), "foo\n\tbar"
test(@(parse) assert_equals(parse(" [ ] ").__string(), [].__string()
test(@(parse) assert_equals(parse(" [ 0 ] ").__string(), [0].__string()
test(@(parse) assert_equals(parse(" [ 0 , 1 ] ").__string(), [0, 1].__string()
test(@(parse) assert_equals(parse(" { } ").__string(), {}.__string()
test(@(parse) assert_equals(parse(" { \"foo\" : 0 } ").__string(), {"foo": 0}.__string()
test(@(parse) assert_equals(parse(" { \"foo\" : 0 , \"bar\" : 1 } ").__string(), {"foo": 0, "bar": 1}.__string()
test(@(parse) assert_equals(parse(" { \"foo\" : [ { } ] , \"bar\" : { } } ").__string(), {"foo": [{}], "bar": {}}.__string()
assert_equals(json.parse(Source([
	0x22,
	0x24,
	0xc2, 0xa3,
	0xe0, 0xa4, 0xb9,
	0xe2, 0x82, 0xac,
	0xf0, 0x90, 0x8d, 0x88,
	0x22
]).read), list_to_string([
	0x24,
	0xc2, 0xa3,
	0xe0, 0xa4, 0xb9,
	0xe2, 0x82, 0xac,
	0xf0, 0x90, 0x8d, 0x88
])
test(@(parse) assert_equals(parse("\"\\ud800\\udf48\\ud801\\udc37\""), list_to_string([
	0xf0, 0x90, 0x8d, 0x88,
	0xf0, 0x90, 0x90, 0xb7
])
